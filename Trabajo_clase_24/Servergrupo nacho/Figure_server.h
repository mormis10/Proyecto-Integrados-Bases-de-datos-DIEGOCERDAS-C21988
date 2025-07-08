#include <atomic> 
#include <iostream>
#include <csignal>
#include <pthread.h>
#include <mutex>
#include <vector>
#include "Socket.h"
#include "File_system.h"
#define BUFSIZE 1024
#define NUM_HILOS 6
#define SERVER_RESPONSE_PORT	5353
#define SERVERS_PORT 8081
#define BROADCAST_WAIT 120
#define TIMEOUT_RESPONSE	5

extern std::atomic<bool> running;

class Figure_server {

  File_system fs;
  std::mutex fs_mutex;
  pthread_t hilos[2];

  public:

  std::string ip;
  std::string name;
  
  Figure_server();
  Figure_server(std::string name, std::string ip);
  /* devuelve un vector de string usando como separador a pattern, pensado para
  la respuesta de "GET /servers" :  ServerName | ip | {lista dibujos}
  */
  std::vector<std::string> split(const std::string& text, 
    const std::string& pattern);
  /* Devuelve la hilera restante en caso de que resulte verdadero, de otra
  forma devuelve nullptr
  */
  const char* startsWith(const char* str, const char* prefix);
  static void* tcp_handler(void* arg);
  static void* tcp_asist(void* arg);
  static void* udp_broadcast(void* arg);
};

struct TCP_args{
  Socket* client;
  Figure_server* Servidor;
};