#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <csignal>
#include <atomic>
#include <ifaddrs.h>
#include <net/if.h>
#include <chrono>

#define DISCOVERY_PORT 5354
#define HTTP_PORT 8080
#define BUFSIZE 4096

std::mutex mtx;
std::map<std::string, std::string> figureMap; // figura -> IP servidor
std::atomic<bool> running{true};

// Función para obtener automáticamente la IP de broadcast
std::string getBroadcastIP() {
   return "172.16.123.111";
}

void signalHandler(int signum) {
    std::cout << "Recibida señal de terminación, cerrando..." << std::endl;
    running = false;
}

// Escucha respuestas de servidores
void listenDiscovery() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Error al crear socket de descubrimiento");
        return;
    }

    // Set socket timeout to prevent blocking indefinitely
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    sockaddr_in recv_addr{};
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(DISCOVERY_PORT);
    recv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("Error al bindear socket de descubrimiento");
        close(sock);
        return;
    }

    char buffer[BUFSIZE];
    while (running) {
        sockaddr_in sender_addr{};
        socklen_t sender_len = sizeof(sender_addr);
        
        ssize_t len = recvfrom(sock, buffer, BUFSIZE - 1, 0, 
                             (sockaddr*)&sender_addr, &sender_len);
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue; // Timeout, continue loop
            }
            if (running) perror("Error al recibir en socket de descubrimiento");
            break;
        }
        buffer[len] = '\0';
        std::string msg(buffer);

        char sender_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip, INET_ADDRSTRLEN);

        if (msg.find("Response:") == 0) {
            std::string name, ip, list;
            std::string content = msg.substr(9); // remove "Response:"
            
            // Parse: Name | IP | figures
            size_t pipe1 = content.find('|');
            size_t pipe2 = content.find('|', pipe1 + 1);
            
            if (pipe1 != std::string::npos && pipe2 != std::string::npos) {
                name = content.substr(0, pipe1);
                ip = content.substr(pipe1 + 1, pipe2 - pipe1 - 1);
                list = content.substr(pipe2 + 1);
                
                // Trim whitespace
                name.erase(0, name.find_first_not_of(" \t"));
                name.erase(name.find_last_not_of(" \t") + 1);
                ip.erase(0, ip.find_first_not_of(" \t"));
                ip.erase(ip.find_last_not_of(" \t") + 1);
                list.erase(0, list.find_first_not_of(" \t"));
                list.erase(list.find_last_not_of(" \t") + 1);

                std::stringstream figs(list);
                std::string fig;
                std::lock_guard<std::mutex> lock(mtx);
                while (getline(figs, fig, ',')) {
                    fig.erase(0, fig.find_first_not_of(" \t"));
                    fig.erase(fig.find_last_not_of(" \t") + 1);
                    if (!fig.empty()) {
                        figureMap[fig] = ip;
                    }
                }
                std::cout << "Servidor registrado: " << name << " @ " << ip 
                         << " con figuras: " << list << std::endl;
            }
        } else if (msg.find("Request: Shutdown") == 0) {
            std::string srvName = msg.substr(strlen("Request: Shutdown "));
            std::cout << "Shutdown recibido: " << srvName << "\n";
            std::lock_guard<std::mutex> lock(mtx);
            for (auto it = figureMap.begin(); it != figureMap.end();) {
                if (it->second.find(srvName) != std::string::npos) {
                    it = figureMap.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    close(sock);
}

// Envía broadcast para descubrir servidores
void sendDiscoveryRequest() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Error al crear socket de broadcast");
        return;
    }

    int broadcastEnable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable))) {
        perror("Error al configurar opción de broadcast");
        close(sock);
        return;
    }

    std::string broadcastIP = getBroadcastIP();
    std::cout << "Usando dirección de broadcast: " << broadcastIP << std::endl;

    sockaddr_in bcast_addr{};
    bcast_addr.sin_family = AF_INET;
    bcast_addr.sin_port = htons(DISCOVERY_PORT);
    inet_pton(AF_INET, broadcastIP.c_str(), &bcast_addr.sin_addr);

    std::string msg = "Request: GET /servers";
    if (sendto(sock, msg.c_str(), msg.size(), 0, 
              (sockaddr*)&bcast_addr, sizeof(bcast_addr)) < 0) {
        perror("Error al enviar broadcast");
    }
    close(sock);
}

// Parse HTTP request to extract the figure name
std::string parseHTTPRequest(const std::string& request) {
    // Look for GET /figurename HTTP/1.1
    size_t get_pos = request.find("GET /");
    if (get_pos == std::string::npos) return "";
    
    size_t start = get_pos + 5; // Skip "GET /"
    size_t end = request.find(" ", start);
    if (end == std::string::npos) return "";
    
    return request.substr(start, end - start);
}

// Forward request to figure server and get response
std::string forwardToServer(const std::string& serverIP, const std::string& figura) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return "HTTP/1.1 500 Internal Server Error\r\n\r\nError creating socket";
    }

    // Set connection timeout
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(1234); // Server port from Server.cc
    if (inet_pton(AF_INET, serverIP.c_str(), &serv.sin_addr) <= 0) {
        close(sock);
        return "HTTP/1.1 500 Internal Server Error\r\n\r\nInvalid server IP";
    }

    if (connect(sock, (sockaddr*)&serv, sizeof(serv)) < 0) {
        close(sock);
        return "HTTP/1.1 502 Bad Gateway\r\n\r\nCannot connect to figure server";
    }

    // Send HTTP request to figure server
    std::string req = "GET /GET/" + figura + " HTTP/1.1\r\nHost: " + serverIP + "\r\n\r\n";
    if (send(sock, req.c_str(), req.size(), 0) < 0) {
        close(sock);
        return "HTTP/1.1 502 Bad Gateway\r\n\r\nError sending request to server";
    }

    // Read response from server
    std::string response;
    char buffer[BUFSIZE];
    ssize_t total_read = 0;
    
    while (total_read < BUFSIZE - 1) {
        ssize_t bytes_read = recv(sock, buffer, BUFSIZE - total_read - 1, 0);
        if (bytes_read <= 0) break;
        
        response.append(buffer, bytes_read);
        total_read += bytes_read;
        
        // Check if we have a complete HTTP response
        if (response.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }
    
    close(sock);
    
    if (response.empty()) {
        return "HTTP/1.1 502 Bad Gateway\r\n\r\nNo response from figure server";
    }
    
    return response;
}

// Servidor HTTP que atiende clientes
void httpServer() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error al crear socket HTTP");
        return;
    }

    // Permitir reuso de dirección
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HTTP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr))) {
        perror("Error al bindear socket HTTP");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 10)) {
        perror("Error al escuchar en socket HTTP");
        close(server_fd);
        return;
    }

    std::cout << "Tenedor escuchando en puerto " << HTTP_PORT << std::endl;

    while (running) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        
        if (client_fd < 0) {
            if (running) perror("Error al aceptar conexión HTTP");
            break;
        }

        // Handle client in separate thread
        std::thread([client_fd]() {
            char buffer[BUFSIZE] = {};
            ssize_t bytes_read = recv(client_fd, buffer, BUFSIZE - 1, 0);
            
            if (bytes_read <= 0) {
                close(client_fd);
                return;
            }

            std::string request(buffer, bytes_read);
            std::string figura = parseHTTPRequest(request);
            
            if (figura.empty()) {
                std::string error = "HTTP/1.1 400 Bad Request\r\n\r\nInvalid request format";
                send(client_fd, error.c_str(), error.size(), 0);
                close(client_fd);
                return;
            }

            std::string response;
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (figureMap.count(figura)) {
                    std::string serverIP = figureMap[figura];
                    response = forwardToServer(serverIP, figura);
                } else {
                    response = "HTTP/1.1 404 Not Found\r\n\r\nFigura no encontrada: " + figura;
                    
                    // Show available figures
                    if (!figureMap.empty()) {
                        response += "\n\nFiguras disponibles: ";
                        bool first = true;
                        for (const auto& pair : figureMap) {
                            if (!first) response += ", ";
                            response += pair.first;
                            first = false;
                        }
                    }
                }
            }

            send(client_fd, response.c_str(), response.size(), 0);
            close(client_fd);
        }).detach();
    }
    close(server_fd);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "Iniciando Tenedor (Load Balancer)..." << std::endl;

    std::thread discovery_thread(listenDiscovery);
    std::thread http_thread(httpServer);

    // Wait a bit for threads to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Send initial discovery request
    sendDiscoveryRequest();
    
    // Periodically rediscover servers
    std::thread periodic_discovery([]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            if (running) {
                sendDiscoveryRequest();
            }
        }
    });

    discovery_thread.join();
    http_thread.join();
    periodic_discovery.join();
    
    return 0;
}