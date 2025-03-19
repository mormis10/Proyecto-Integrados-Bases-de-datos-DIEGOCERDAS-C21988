/**
 *  Universidad de Costa Rica
 *  ECCI
 *  CI0123 Proyecto integrador de redes y sistemas operativos
 *  2025-i
 *  Grupos: 1 y 3
 *
 ****** VSocket base class implementation
 *
 * (Fedora version)
 *
 **/

#include <arpa/inet.h> // ntohs, htons
#include <cerrno>
#include <cstdio>
#include <cstring> // memset
#include <iostream>
#include <netdb.h>   // getaddrinfo, freeaddrinfo
#include <stdexcept> // runtime_error
#include <sys/socket.h>
#include <unistd.h> // close
/*
#include <cstddef>
#include <cstdio>

//#include <sys/types.h>
*/
#include "VSocket.h"

/**
 *  Class creator (constructor)
 *     use Unix socket system call
 *
 *  @param     char t: socket type to define
 *     's' for stream
 *     'd' for datagram
 *  @param     bool ipv6: if we need a IPv6 socket
 *
 **/
void VSocket::BuildSocket(char t, bool IPv6) {
  this->IPv6 = IPv6;
  this->type = t;
  int domain;
  int socket_type = SOCK_STREAM;
  if (IPv6) {
    domain = AF_INET6;
    return;
  }
  // tenemos que ver el tipo de dominio, cuando es de tipo IPv6 utilizamos el
  // dominio AF_INET6 y en caso de IPv4 utilizamos AF_INET
  domain = AF_INET;
  // debería de utilizar más el operador terniario

  if (socket_type == -1) {
    throw std::runtime_error("VSocket::BuildSocket - Tipo de socket inválido");
  }

  // Ahora utilizo la biblioteca para crear el socket
  //  para crear el socket y obtener su id ocupo el dominio y el tipo de socket
  this->idSocket = socket(domain, socket_type, 0);

  if (this->idSocket == -1) {
    throw std::runtime_error("VSocket::BuildSocket - Error al crear el socket");
  } else {
    printf("Hecho con éxito nachito\n");
  }
}

/**
 * Class destructor
 *
 **/
VSocket::~VSocket() { this->Close(); }

/**
 * Close method
 *    use Unix close system call (on s.Read( a, 512 );ce opened a socket is managed like a file in
 *Unix)
 *
 **/
void VSocket::Close() {
  if (this->idSocket != -1) {
    // busca el id dl socket y lo cierra
    if (close(this->idSocket) == -1) {
      throw std::runtime_error("VSocket::Close()");
    }
    printf("Cierre realizado con éxito\n");
    this->idSocket = -1;
  } else {
    //throw std::runtime_error("VSocket::Close()");
  }
}

/**
 * EstablishConnection method
 *   use "connect" Unix system call
 *
 * @param      char * host: host address in dot notation, example "10.84.166.62"
 * @param      int port: process address, example 80
 *
 **/
int VSocket::EstablishConnection(const char *hostip, int port) {
  int st = -1;
  this->port = port;
  struct sockaddr_in host4;
  memset((char *)&host4, 0, sizeof(host4));
  host4.sin_family = AF_INET;
  // Guarda aquí el id del host en binairo
  st = inet_pton(AF_INET, hostip, &host4.sin_addr);
  if (-1 == st) {
    throw(std::runtime_error("VSocket::DoConnect, inet_pton"));
  }
  // Asigna el número del puerto a la estrucutra
  host4.sin_port = htons(port);
  st = connect(this->idSocket, (sockaddr *)&host4, sizeof(host4));
  if (-1 == st) {
    perror("Error en connect");
    throw(std::runtime_error("VSocket::DoConnect, connect"));
  }

  if (-1 == st) {
    throw std::runtime_error("VSocket::EstablishConnection");
  }

  return st;
}

int VSocket::EstablishConnection(const char *hostip, int port, bool prove) {
  struct sockaddr_in6 host6;
  struct sockaddr *ha;
  int st;
  memset(&host6, 0, sizeof(host6));
  host6.sin6_family = AF_INET6;
  st = inet_pton(AF_INET6, hostip, &host6.sin6_addr);
  if (0 <= st) { // 0 means invalid address, -1 means address error
    throw std::runtime_error(
        "Socket::Connect( const char *, int ) [inet_pton]");
  }
  host6.sin6_port = htons(port);
  ha = (struct sockaddr *)&host6;
  size_t len;
  len = sizeof(host6);
  st = connect(this->idSocket, ha, len);
  if (-1 == st) {
    throw std::runtime_error("Socket::Connect( const char *, int ) [connect]");
  }
  return st;
}
/**
  * EstablishConnection method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example
"os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  * s.Read( a, 512 );

**/
int VSocket::EstablishConnection(const char *host, const char *service) {
  int st = -1;
  struct addrinfo hints, *res, *p;
  // para esto tenemos que con hints es la estrucutra par definir el tipo de
  // conexión que se desea
  // res va a tener la lista de direcciones y con p vamos a recorrer las
  // direcciones obtenidas

  // esto ya lo sabemos solo estamos añadiendo ceros dentro de hints para
  // limpiarlo en caso de que hubiera basura
  memset(&hints, 0, sizeof(struct addrinfo));

  // Asignamos AF_INET6 para indicarle que deseamos solamente conexiones de tipo
  // Ipv6
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  // Aquí específicamos que queremos un socket TCP

  // busvamos la dirección del servidor
  int status;
  status = getaddrinfo(host,"8080", &hints, &res);
  if (status != 0) {
    std::cerr << "Error en getaddrinfo: " << gai_strerror(status) << std::endl;
  }

  // Ahora sí vamos a recorrer el arrego de direcciones que tenemos
  //  lo recorremos cómo sí fuera una lista enlazada

  for (p = res; p != nullptr; p = p->ai_next) {
    // creamos el socket
    //  obtenemos el id del socket
    std::cout << p->ai_protocol << "\n";
    this->idSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    // si la conexión falla, continue con la siguiente
    if (this->idSocket == -1) {
      continue;
    }
    // en caso de que sí fuera exitoso
    // en ese caso sí nos conectamos

    // std::cout<<p->ai_addr;
    st = connect(this->idSocket, p->ai_addr, p->ai_addrlen);
    // en caso de que la conexión se haya realizado de manera correcta
    // dejamos de recorrer el arreglo
    if (st == 0) {
      break;
    }
    // en caso de que algo haya pasado mal, entonces cerramos el socket
    close(this->idSocket);
  }
  // liberamos la memoria dinámica alojada en res
  freeaddrinfo(res);

  if (st == -1) {
    throw std::runtime_error("VSocket::EstablishConnection");
  }

  return st;
}

/**
  * Bind method
  *    use "bind" Unix system call (man 3 bind) (server mode)
  *
  * @param      int port: bind a unamed socket to a port defined in sockaddr structure
  *
  *  Links the calling process to a service at port
  *
 **/
int VSocket::Bind( int port ) {
   int st = -1;

   return st;

}


/**
  *  sendTo method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to send data
  *
  *  Send data to another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::sendTo( const void * buffer, size_t size, void * addr ) {
   int st = -1;

   return st;

}


/**
  *  recvFrom method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to receive from data
  *
  *  @return	size_t bytes received
  *
  *  Receive data from another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::recvFrom( void * buffer, size_t size, void * addr ) {
   int st = -1;

   return st;

}
