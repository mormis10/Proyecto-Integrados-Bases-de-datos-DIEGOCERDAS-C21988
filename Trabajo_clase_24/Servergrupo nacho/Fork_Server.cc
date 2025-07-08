#include <vector>
#include <string>
#include <mutex>
#include <cstring>      
#include <thread>       
#include <unistd.h>     
#include <ctime>        
#include <exception>    
#include <pthread.h>    
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Fork_Server.h"

std::atomic<bool> runningF(true);

std::vector<std::string> Fork_server::broadcast_addresses = {
  "172.16.123.15",
  "172.16.123.31",
  "172.16.123.47",
  "172.16.123.79",
  "172.16.123.95",
  "172.16.123.111",
};

Fork_server::Fork_server(){
  std::cout << "[INIT] Iniciando Fork_server...\n";

  pthread_create(&hilos[0], nullptr, Fork_server::udp_discovery, this);
  pthread_create(&hilos[1], nullptr, Fork_server::udp_client_receptor, this);

  for (int i = 0; i < 2; ++i) {
    pthread_join(hilos[i], nullptr);
  }
}

/* devuelve un vector de string usando como separador a pattern, pensado para
la respuesta de "GET /servers" :  ServerName | ip | {lista dibujos}
*/
std::vector<std::string> Fork_server::split(const std::string& text, const std::string& pattern){
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

const char* Fork_server::startsWith(const char* str, const char* prefix){
  size_t len_prefix = std::strlen(prefix);
  if (std::strncmp(str, prefix, len_prefix) == 0) {
    return str + len_prefix;
  }
  return nullptr;
}

void Fork_server::addRoute(const std::string& fullResponse) {
  auto campos = split(fullResponse, "\\s*\\|\\s*");
  if (campos.size() != 3) return;

  std::string serverName = campos[0];
  std::string ip = campos[1];
  auto figuras = split(campos[2], "\\s*,\\s*");

  std::cout << "[ADDROUTE] Agregando servidor: " << serverName << " con IP: " << ip << "\n";
  Direction t{serverName, ip};

  std::lock_guard<std::mutex> lock(tabla_mutex);
  for (const std::string& fig : figuras) {
    tabla[fig] = t;
    std::cout << "  └── Figura registrada: " << fig << "\n";
  }

  if (sockets.find(ip) == sockets.end() || !sockets[ip]) {
    Socket* tcp_socket;
    try {
      tcp_socket = new Socket('s');
      tcp_socket->MakeConnection(ip.c_str(), SERVERS_PORT);
    } catch(const std::exception& e) {
      tcp_socket = nullptr;
    }
    sockets[ip] = tcp_socket;
    if (sockets[ip]) {
      std::cout << "  └── Socket creado y registrado para IP: " << ip << "\n";
    } else {
      std::cout << "  └── No se logro conexion TCP, espere hasta el siguiente broadcast: " << ip << "\n";
    }
    
    
  } else {
    std::cout << "  └── Ya existe un socket para IP: " << ip << ", no se sobreescribe.\n";
  }
}

bool Fork_server::searchRoute(const std::string& clave, Direction& resultado) {
  std::lock_guard<std::mutex> lock(tabla_mutex);
  auto it = tabla.find(clave);
  if (it != tabla.end()) {
    resultado = it->second;
    return true;
  }
  return false;
}

void* Fork_server::udp_client_receptor(void* arg) {
  Fork_server* self = static_cast<Fork_server*>(arg);

  try {
    Socket udp_socket('d'); 
    udp_socket.Bind(CLIENT_PORT);
    std::cout << "[UDP CLIENT] Receptor iniciado en puerto " << CLIENT_PORT << "\n";
    int sockfd = udp_socket.getIDSocket();

    while (runningF) {
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(sockfd, &fds);
      struct timeval timeout{};
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      int ready = select(sockfd + 1, &fds, nullptr, nullptr, &timeout);
      if (ready > 0 && FD_ISSET(sockfd, &fds)){
      sockaddr_in client_addr{};
      std::memset(&client_addr, 0, sizeof(client_addr));
      char* buffer = new char[BUFSIZE];
      std::memset(buffer, 0, BUFSIZE);
      ssize_t recv_len = udp_socket.recvFrom(buffer, BUFSIZE, &client_addr);

      std::cout << "[UDP CLIENT] Solicitud recibida: " << buffer << "\n";

      const char* name = self->startsWith(buffer, "GET /figure/");
      if (!name) {
        std::cout << "[UDP CLIENT] Formato no reconocido. Ignorando...\n";
        delete[] buffer;
        continue;
      }

      std::vector<std::string> parts = self->split(name, "[\\r\\n]");
      std::string strName = parts.empty() ? "" : parts[0];
      std::cout << "[UDP CLIENT] Buscando figura: " << strName << "\n";

      Direction resultado;
      if (!self->searchRoute(strName, resultado)) {
        std::cout << "[UDP CLIENT] Figura no encontrada\n";

        std::string not_found = "HTTP/1.1 404 Not Found\r\n\r\nFigura no encontrada\n";
        udp_socket.sendTo(not_found.c_str(), not_found.length(), &client_addr);

        delete[] buffer;
        continue;
      }

      argsTCP* args = new argsTCP;
      args->client = client_addr;
      args->ip = resultado.ip;
      args->message = new char[recv_len];
      std::memset(args->message, 0, recv_len);
      args->message_len = recv_len;
      args->server = self;

      std::memcpy(args->message, buffer, recv_len);

      std::cout << "[UDP CLIENT] Solicitud a reenviar: " << args->message << "\n";
      fflush(stdout); 
      delete[] buffer;

      std::cout << "[UDP CLIENT] Redirigiendo solicitud al servidor TCP en " << resultado.ip << "\n";
      pthread_t tid;
      pthread_create(&tid, nullptr, Fork_server::tcp_servers, args);
      printf("Acaba2\n");
      pthread_detach(tid);
    }

    }

  } catch (const std::exception& e) {
    std::cerr << "[UDP CLIENT] Excepción: " << e.what() << std::endl;
  }
  return nullptr;
}

void* Fork_server::tcp_servers(void* arg) {
  try {
    argsTCP* args = static_cast<argsTCP*>(arg);

    std::cout << "[TCP SERVER] Solicitud a enviar: " << args->message << "\n";
    fflush(stdout);

    // Recuperamos el SafeSocket existente de la tabla
    auto& socket_map = args->server->sockets;
    auto it = socket_map.find(args->ip);
    if (it == socket_map.end() || !socket_map[args->ip]) {
      std::cerr << "[TCP SERVER] No hay socket registrado para IP: " << args->ip << "\n";
      delete[] args->message;
      delete args;
      return nullptr;
    }

    std::string respuesta = "";
    int len = 0;

    Socket& s = *socket_map[args->ip];
    std::cout << "[TCP SERVER] Usando socket ya conectado a: " << args->ip << "\n";
    std::cout << "[TCP SERVER] Enviando solicitud al servidor...\n";
    s.Write(args->message, args->message_len);

    // Lectura de la respuesta
    bool option = true;
    int total = 0;
    char buffer[BUFSIZE];
    do{
     memset(buffer,0,512);
     len = s.Read(buffer, BUFSIZE);
     if(len == BUFSIZE){
      option = true;
     }else{
      option = false;
     }
     std::cout<<"Entra en ciclo\n";
     std::cout<<len <<"\n";
     total+= len;
     respuesta+= std::string(buffer);
    }while(option);
  

    std::cout << "[TCP SERVER] Respuesta recibida de servidor: " << respuesta << "\n";

    Socket udp_socket('d');
    udp_socket.sendTo(respuesta.c_str(), total, &args->client);
    std::cout << "[TCP SERVER] Respuesta enviada al cliente UDP\n";

    // Limpieza de argumentos
    delete[] args->message;
    delete args;

  } catch (const std::exception& e) {
    std::cerr << "[TCP SERVER] Excepción: " << e.what() << std::endl;
  }
  printf("Acaba\n");

  return nullptr;
}

void* Fork_server::udp_discovery(void* arg) {
  Fork_server* self = static_cast<Fork_server*>(arg);

  try {
    Socket udp_socket('d');

    int broadcast = 1;
    setsockopt(udp_socket.getIDSocket(), SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    udp_socket.Bind(SERVER_DISCOVERY_PORT);
    std::cout << "[DISCOVERY] Servidor de descubrimiento iniciado en puerto " << SERVER_DISCOVERY_PORT << "\n";

    const char* discovery_msg = "GET /servers";
    int sockfd = udp_socket.getIDSocket();

    while (runningF) {
      for (const std::string& ip : broadcast_addresses) {
        std::cout << "[DISCOVERY] Enviando broadcast a: " << ip << "\n";
        sockaddr_in dest{};
        dest.sin_family = AF_INET;
        dest.sin_port = htons(SERVER_DISCOVERY_PORT);
        inet_pton(AF_INET, ip.c_str(), &dest.sin_addr);
        udp_socket.sendTo(discovery_msg, strlen(discovery_msg), &dest);
      }

      int responses_received = 0;
      time_t start_time = time(nullptr);

      while (responses_received < NUM_HILOS &&
             time(nullptr) - start_time < TIMEOUT_RESPONSE &&
             runningF) {

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ready = select(sockfd + 1, &fds, nullptr, nullptr, &tv);

        if (ready > 0 && FD_ISSET(sockfd, &fds)) {
          char buffer[BUFSIZE];
          sockaddr_in sender{};
          ssize_t len = udp_socket.recvFrom(buffer, BUFSIZE, &sender);

          if (len > 0) {
            buffer[len] = '\0';
            std::string response(buffer);
            std::cout << "[DISCOVERY] Respuesta recibida: " << response << "\n";
            self->addRoute(response);
            ++responses_received;
          }
        }
      }

      for (int i = 0; i < BROADCAST_WAIT && runningF; ++i) {
        sleep(1);
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "[DISCOVERY] Excepción: " << e.what() << std::endl;
  }

  return nullptr;
}

