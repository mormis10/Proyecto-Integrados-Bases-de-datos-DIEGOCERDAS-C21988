#include <netinet/in.h>
#include <sys/socket.h>
#include "Figure_server.h"
#include <regex>

std::atomic<bool> running(true);

Figure_server::Figure_server() {}

Figure_server::Figure_server(std::string name, std::string ip) : ip(ip), name(name) {
  std::cout << "[INIT] Iniciando Figure_server con nombre: " << name << ", IP: " << ip << std::endl;

  pthread_create(&hilos[0], nullptr, Figure_server::udp_broadcast, this);
  pthread_create(&hilos[1], nullptr, Figure_server::tcp_handler, this);

  for (int i = 0; i < 2; ++i) {
    pthread_join(hilos[i], nullptr);
  }
}

std::vector<std::string> Figure_server::split(const std::string& text, const std::string& pattern) {
  std::regex spliter(pattern);
  std::sregex_token_iterator iter(text.begin(), text.end(), spliter, -1);
  std::sregex_token_iterator end;

  std::vector<std::string> result;
  while (iter != end) {
    if (!iter->str().empty()) {
      result.push_back(*iter);
    }
    ++iter;
  }

  return result;
}

const char* Figure_server::startsWith(const char* str, const char* prefix) {
  size_t len_prefix = std::strlen(prefix);
  if (std::strncmp(str, prefix, len_prefix) == 0) {
    return str + len_prefix;
  }
  return nullptr;
}

void* Figure_server::tcp_handler(void* arg) {
  Figure_server* self = static_cast<Figure_server*>(arg);

  try {
    Socket listener('s');
    listener.Bind(SERVERS_PORT);
    listener.Listen();

    std::cout << "[TCP HANDLER] Servidor TCP escuchando en puerto " << SERVERS_PORT << std::endl;
    int sockfd = listener.getIDSocket();

    while (running) {
     fd_set fds;
     FD_ZERO(&fds);
     FD_SET(sockfd, &fds);
     struct timeval timeout{};
     timeout.tv_sec = 1;
     timeout.tv_usec = 0;
     int ready = select(sockfd + 1, &fds, nullptr, nullptr, &timeout);
     if (ready > 0 && FD_ISSET(sockfd, &fds)) {
      Socket* client = listener.Accept();
      std::cout << "[TCP HANDLER] Conexión aceptada" << std::endl;

      TCP_args* args = new TCP_args;
      args->client = client;
      args->Servidor = self;

      pthread_t tid;
      pthread_create(&tid, nullptr, Figure_server::tcp_asist, args);
      pthread_detach(tid);
     }
    }
  } catch (const std::exception& e) {
    std::cerr << "[TCP HANDLER] Excepción: " << e.what() << std::endl;
  }

  return nullptr;
}

void* Figure_server::tcp_asist(void* arg) {
  TCP_args* args = static_cast<TCP_args*>(arg);
  Socket* client = args->client;
  Figure_server* server = args->Servidor;
  delete args;

  std::cout << "[TCP ASIST] Hilo iniciado para nueva conexión\n";

  try {
    while (running) {
      char buffer[BUFSIZE];
      
      ssize_t len = client->Read(buffer, BUFSIZE);
      if (len <= 0) {
        std::cerr << "[TCP ASIST] Cliente desconectado o lectura vacía, cerrando sesión\n";
        break;  
      }
      buffer[len] = '\0';
      std::cout << "[TCP ASIST] Mensaje recibido: " << buffer << std::endl;

      const char* figureName = server->startsWith(buffer, "GET /figure/");
      if (!figureName) {
        std::cerr << "[TCP ASIST] Formato inválido o no es GET /figure/, ignorando\n";
        continue;  
      }
      
      std::string requested(figureName);
      if (auto pos = requested.find(' '); pos != std::string::npos) {
        requested.resize(pos);
      }

      char* content = nullptr;
      {
        std::lock_guard<std::mutex> lock(server->fs_mutex);
        content = server->fs.read_file(requested.c_str());
      }
      std::cout << "[TCP ASIST] Figura solicitada: " << requested << std::endl;

      if (content) {
        std::string response = "HTTP/1.1 200 OK\r\n\r\n" + std::string(content);
        client->Write(response.c_str(), response.size());
        delete[] content;
      } else {
        const char* not_found =
            "HTTP/1.1 404 Not Found\r\n\r\nFigura no encontrada\n";
        client->Write(not_found, strlen(not_found));
      }

    }
  } catch (const std::exception& e) {
      std::cerr << "[TCP ASIST] Excepción: " << e.what() << std::endl;
  }


  delete client;
  return nullptr;
}

void* Figure_server::udp_broadcast(void* arg) {
  Figure_server* self = static_cast<Figure_server*>(arg);

  try {
    Socket udp_socket('d');
    udp_socket.Bind(SERVER_RESPONSE_PORT);

    std::cout << "[UDP BROADCAST] Esperando solicitudes en puerto " << SERVER_RESPONSE_PORT << std::endl;

    char buffer[BUFSIZE];
     int sockfd = udp_socket.getIDSocket();

    while (running) {
     fd_set fds;
     FD_ZERO(&fds);
     FD_SET(sockfd, &fds);
     struct timeval timeout{};
     timeout.tv_sec = 1;
     timeout.tv_usec = 0;
     int ready = select(sockfd + 1, &fds, nullptr, nullptr, &timeout);
     if (ready > 0 && FD_ISSET(sockfd, &fds)) {
      struct sockaddr_in sender_addr;
      memset( &sender_addr, 0, sizeof( sender_addr ) ); 
      ssize_t len = udp_socket.recvFrom(buffer, BUFSIZE, &sender_addr);
      if (len <= 0) continue;

      buffer[len] = '\0';
      std::cout << "[UDP BROADCAST] Mensaje recibido: " << buffer << std::endl;

      const char* rest = self->startsWith(buffer, "GET /servers");
      if (!rest) {
        std::cout << "[UDP BROADCAST] Solicitud ignorada" << std::endl;
        continue;
      }

      std::string serverName = self->name;
      std::string serverIP = self->ip;

      char* figuras_cstr = self->fs.list_file();
      std::string figuras_list = figuras_cstr ? std::string(figuras_cstr) : "";

      if (figuras_cstr) {
        delete[] figuras_cstr;
      }

      std::string response = serverName + " | " + serverIP + " | " + figuras_list;
      std::cout << "[UDP BROADCAST] Enviando respuesta: " << response << std::endl;

      udp_socket.sendTo(response.c_str(), response.size(), &sender_addr);
    }

    }

  } catch (const std::exception& e) {
    std::cerr << "[UDP BROADCAST] Excepción: " << e.what() << std::endl;
  }

  return nullptr;
}