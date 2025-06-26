#include <iostream>
#include <thread>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <atomic>
#include "FileSystem.h"
#include "VSocket.h"
#include "Socket.h"

#define PORT 1234
#define BUFSIZE 4096
#define DISCOVERY_PORT 5354

std::string serverName = "ServidorA";
FileSystem* globalFs = nullptr;
std::atomic<bool> running{true};

// Get local IP address automatically
std::string getLocalIP() {
    struct ifaddrs *ifaddr, *ifa;
    std::string localIP = "127.0.0.1"; // fallback

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return localIP;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        // Skip loopback interface
        if (ifa->ifa_flags & IFF_LOOPBACK)
            continue;
            
        // Get interface that's up and running
        if (ifa->ifa_flags & IFF_UP && ifa->ifa_flags & IFF_RUNNING) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            localIP = inet_ntoa(sa->sin_addr);
            break;
        }
    }

    freeifaddrs(ifaddr);
    return localIP;
}

// Get broadcast IP for the network
std::string getBroadcastIP() {
  return "172.16.123.111";
}

/**
 * Handles a client request in a separate thread.
 */
void task(VSocket *client, FileSystem& fs) {
    char buffer[BUFSIZE] = {};
    ssize_t bytes_read = client->Read(buffer, BUFSIZE - 1);
    
    if (bytes_read <= 0) {
        client->Close();
        return;
    }
    
    std::string request(buffer, bytes_read);
    std::string response;

    std::cout << "Received request: " << request.substr(0, 100) << "..." << std::endl;

    // Check if the request starts with "GET"
    if (request.substr(0, 4) == "GET ") {
        size_t pos1 = request.find("GET ");
        size_t pos2 = request.find(" HTTP");

        if (pos1 != std::string::npos && pos2 != std::string::npos) {
            // Extract the requested URL path
            std::string url = request.substr(pos1 + 4, pos2 - (pos1 + 4));

            // Remove carriage return and newline characters
            url.erase(std::remove(url.begin(), url.end(), '\r'), url.end());
            url.erase(std::remove(url.begin(), url.end(), '\n'), url.end());

            std::cout << "Parsed URL: '" << url << "'" << std::endl;

            // Handle GET /LIST command to list available figures
            if (url == "/LIST") {
                std::string list = fs.getFigureList();
                response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nConnection: close\r\n\r\n" + list;
            } else {
                // Handle GET /GET/figureName or GET /figureName
                std::string figureName = url;
                if (url.find("/GET/") == 0) {
                    figureName = url.substr(5);  // Remove "/GET/" prefix
                } else if (url.find("/") == 0) {
                    figureName = url.substr(1);   // Remove "/" prefix
                }

                std::cout << "Looking for figure: '" << figureName << "'" << std::endl;

                std::string figure = fs.getFigureContent(figureName);

                // Check if figure was found
                if (!figure.empty() && figure != "Figure not found.\n") {
                    response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nConnection: close\r\n\r\n" + figure;
                    std::cout << "Figure found and sent: " << figureName << std::endl;
                } else {
                    response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nFigure not found: " + figureName;
                    std::cout << "Figure not found: " << figureName << std::endl;
                }
            }
        }
    } else {
        // Handle unsupported or malformed request
        response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nInvalid command";
    }

    // Send response and close connection
    client->Write(response.c_str(), response.size());
    client->Close();
}

/**
 * Devuelve los nombres de figuras separados por coma
 */
std::string getFigureNamesCSV(FileSystem& fs) {
    std::ostringstream oss;
    DirectoryBlock dir;
    int blockNum = 0;
    bool first = true;

    while (true) {
        fs.readBlock(blockNum, reinterpret_cast<char*>(&dir));
        for (int i = 0; i < DIR_ENTRIES; ++i) {
            if (dir.entries[i].name[0] != '\0') {
                if (!first) oss << ",";
                oss << dir.entries[i].name;
                first = false;
            }
        }
        if (dir.nextBlock == 0) break;
        blockNum = dir.nextBlock;
    }

    return oss.str();
}

/**
 * Listen for discovery requests
 */
void listenForDiscovery(FileSystem& fs) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Error creating discovery socket");
        return;
    }

    // Set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVERY_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Error binding discovery socket");
        close(sock);
        return;
    }

    std::cout << "Listening for discovery requests on port " << DISCOVERY_PORT << std::endl;

    char buffer[BUFSIZE];
    while (running) {
        sockaddr_in sender_addr{};
        socklen_t sender_len = sizeof(sender_addr);
        
        ssize_t len = recvfrom(sock, buffer, BUFSIZE - 1, 0, 
                             (sockaddr*)&sender_addr, &sender_len);
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue; // Timeout, continue
            }
            if (running) perror("Error receiving discovery request");
            break;
        }
        
        buffer[len] = '\0';
        std::string msg(buffer);

        if (msg.find("Request: GET /servers") == 0) {
            std::cout << "Discovery request received" << std::endl;
            
            std::string localIP = getLocalIP();
            std::string figureList = getFigureNamesCSV(fs);
            
            // Send response
            std::string response = "Response:" + serverName + "|" + localIP + "|" + figureList;
            
            if (sendto(sock, response.c_str(), response.size(), 0, 
                      (sockaddr*)&sender_addr, sizeof(sender_addr)) < 0) {
                perror("Error sending discovery response");
            } else {
                std::cout << "Discovery response sent: " << response << std::endl;
            }
        }
    }
    
    close(sock);
}

/**
 * Envía anuncio del servidor por UDP broadcast
 */
void announceServer(const std::string& name, const std::string& ip, const std::string& figures) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Error creating broadcast socket");
        return;
    }
    
    int bcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &bcast, sizeof(bcast));

    std::string broadcastIP = getBroadcastIP();
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVERY_PORT);
    inet_pton(AF_INET, broadcastIP.c_str(), &addr.sin_addr);

    std::string msg = "Response:" + name + "|" + ip + "|" + figures;
    if (sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Error sending broadcast");
    } else {
        std::cout << "Broadcast sent: " << msg << " to " << broadcastIP << std::endl;
    }
    
    close(sock);
}

/**
 * Envía notificación de apagado del servidor
 */
void shutdownBroadcast(const std::string& name) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Error creating shutdown broadcast socket");
        return;
    }
    
    int bcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &bcast, sizeof(bcast));

    std::string broadcastIP = getBroadcastIP();
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVERY_PORT);
    inet_pton(AF_INET, broadcastIP.c_str(), &addr.sin_addr);

    std::string msg = "Request: Shutdown " + name;
    if (sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Error sending shutdown broadcast");
    } else {
        std::cout << "Shutdown broadcast sent: " << msg << std::endl;
    }
    
    close(sock);
}

/**
 * Manejador de señal SIGINT para apagar el servidor limpiamente
 */
void handleSigint(int) {
    std::cout << "\nShutting down server..." << std::endl;
    running = false;
    shutdownBroadcast(serverName);
    exit(0);
}

/**
 * Main del servidor
 */
int main() {
    std::signal(SIGINT, handleSigint);

    FileSystem fs;
    globalFs = &fs;

    int figureCount = fs.countFigures("ASCII");
    int neededDirBlocks = (figureCount + DIR_ENTRIES - 1) / DIR_ENTRIES;

    if (!std::filesystem::exists(DISK_FILENAME)) {
        fs.initDisk(neededDirBlocks);
    }

    fs.loadFiguresFromFolder("ASCII");

    // Get local IP and announce server
    std::string localIP = getLocalIP();
    std::string figureList = getFigureNamesCSV(fs);
    
    std::cout << "Server IP: " << localIP << std::endl;
    std::cout << "Available figures: " << figureList << std::endl;
    
    announceServer(serverName, localIP, figureList);

    // Start discovery listener thread
    std::thread discovery_thread(listenForDiscovery, std::ref(fs));

    // Start main server
    VSocket *s1, *client;
    s1 = new Socket('s', false);  // Use IPv4 for compatibility

    s1->Bind(PORT);
    s1->MarkPassive(10);

    std::cout << "Server listening on port " << PORT << ". Waiting for connections..." << std::endl;

    while (running) {
        try {
            client = s1->AcceptConnection();
            if (client) {
                std::thread(task, client, std::ref(fs)).detach();
            }
        } catch (const std::exception& e) {
            if (running) {
                std::cerr << "Error accepting connection: " << e.what() << std::endl;
            }
            break;
        }
    }

    discovery_thread.join();
    return 0;
}
