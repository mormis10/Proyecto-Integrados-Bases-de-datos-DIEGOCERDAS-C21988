#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Socket.h"

#define BUFSIZE 512
#define FORK_PORT 8080

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Uso: " << argv[0] << " <nombre_figura>" << std::endl;
    return EXIT_FAILURE;
  }

  std::string figura = argv[1];
  std::string request = "GET /figure/" + figura;

  // Crear socket UDP
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("socket");
    return EXIT_FAILURE;
  }

  // Dirección del fork
  sockaddr_in fork_addr{};
  fork_addr.sin_family = AF_INET;
  fork_addr.sin_port = htons(FORK_PORT);
  inet_pton(AF_INET, "172.16.123.102", &fork_addr.sin_addr);

  // Enviar solicitud
  ssize_t sent = sendto(sockfd, request.c_str(), request.size(), 0,
                        reinterpret_cast<sockaddr*>(&fork_addr), sizeof(fork_addr));
  if (sent < 0) {
    perror("sendto");
    close(sockfd);
    return EXIT_FAILURE;
  }

  std::cout << "[CLIENTE] Solicitud enviada: " << request << std::endl;

  // Recibir respuesta
  char buffer[BUFSIZE];
  sockaddr_in server_addr{};
  socklen_t addr_len = sizeof(server_addr);
  bool option = true;
  ssize_t recv_len = 0;
  std::string response_message = "";
  do{
   memset(buffer,0,512);
   recv_len = recvfrom(sockfd, buffer, BUFSIZE - 1, 0,
                              reinterpret_cast<sockaddr*>(&server_addr), &addr_len);
                          
   if (recv_len < 0) {
    perror("recvfrom");
    close(sockfd);
    return EXIT_FAILURE;
  }
  response_message+= std::string(buffer);
  if(recv_len != 0){
    option = false;
  }

  }while(option);
  
  std::cout << "[CLIENTE] Respuesta recibida:\n" << response_message << std::endl;

  close(sockfd);
  return EXIT_SUCCESS;
}