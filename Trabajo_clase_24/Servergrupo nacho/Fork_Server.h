#ifndef FORK_SERVER_H
#define FORK_SERVER_H
#include <atomic> 
#include <csignal>
#include <iostream> 
#include <unordered_map>
#include <mutex>
#include <regex>
#include <pthread.h>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include "Socket.h"
#define NUM_HILOS 6
#define NUM_SERVER 5
#define SERVER_DISCOVERY_PORT	5353
#define CLIENT_PORT 8080
#define SERVERS_PORT 8081
#define BUFSIZE 512
#define BROADCAST_WAIT 120
#define TIMEOUT_RESPONSE	5

/*struct pensado para el hashmap*/
struct Direction {
  std::string name;
  std::string ip;
};

extern std::atomic<bool> runningF;


class Fork_server {
  static std::vector<std::string> broadcast_addresses;
  std::unordered_map<std::string, Direction> tabla;
  std::unordered_map<std::string, Socket*> sockets;
  std::mutex tabla_mutex;
  pthread_t hilos[2];

  public:
  
  Fork_server();
  /* devuelve un vector de string usando como separador a pattern, pensado para
  la respuesta de "GET /servers" :  ServerName | ip | {lista dibujos}
  */
  std::vector<std::string> split(const std::string& text, 
    const std::string& pattern);
  /* Devuelve la hilera restante en caso de que resulte verdadero, de otra
  forma devuelve nullptr
  */
  const char* startsWith(const char* str, const char* prefix);
  /*añade Directions al hashmap para ruteo*/
  void addRoute(const std::string& fullResponse);
  /*buscar figura y por tanto Direction con ip y el nombre del server*/
  bool searchRoute(const std::string& clave, Direction& resultado);
  static void* tcp_servers(void* arg);
  static void* udp_client_receptor(void* arg);
  static void* udp_discovery(void* arg);
};

struct argsTCP {
  sockaddr_in client;
  char* message;
  size_t message_len;
  std::string ip;
  Fork_server* server;
};
#endif