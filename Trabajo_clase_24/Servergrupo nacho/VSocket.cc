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

 
#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>			// getaddrinfo, freeaddrinfo
#include <unistd.h>			// close
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
 void VSocket::BuildSocket( char t, bool IPv6 ){
  int domain;
  if (IPv6) {
    domain = AF_INET6;
  } else {
    domain = AF_INET;
  }
  
  if (t == 's') {
    this -> idSocket = socket(domain, SOCK_STREAM, 0);
  } else {
    this -> idSocket = socket(domain, SOCK_DGRAM, 0);
  }

  if (this->idSocket == -1) {
    throw std::runtime_error("Socket creation failed");
  }
  
   this -> IPv6 = IPv6;
   this -> type = t;
 }
 
 
 /**
   * Class destructor
   *
  **/
 VSocket::~VSocket() {
    this->Close();
 }
 
 
 /**
   * Close method
   *    use Unix close system call (once opened a socket is managed like a file in Unix)
   *
  **/
 void VSocket::Close(){
    close(this->idSocket);
 }

 int VSocket::getIDSocket() {
  return idSocket;
 }
 
 /**
   * EstablishConnection method
   *   use "connect" Unix system call
   *
   * @param      char * host: host address in dot notation, example "10.84.166.62"
   * @param      int port: process address, example 80
   *
  **/
 int VSocket::EstablishConnection( const char * hostip, int port ) {
  if (IPv6) {
    sockaddr_in6 serverAddress;
    memset( &serverAddress, 0, sizeof( serverAddress ) );
    serverAddress.sin6_family = AF_INET6;
    int st = inet_pton( AF_INET6, hostip, &serverAddress.sin6_addr );

    if ( st == -1 ) {	// 0 means invalid address, -1 means address error
      throw std::runtime_error( "Socket::Connect( const char *, int ) [inet_pton]" );
    }

    serverAddress.sin6_port = htons( port );

    st = connect(this -> idSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if ( -1 == st ) {
      throw std::runtime_error( "Socket::Connect( const char *, int ) [connect]" );
    }

  } else {

    sockaddr_in serverAddress;
    memset( &serverAddress, 0, sizeof( serverAddress ) );
    serverAddress.sin_family = AF_INET;
    int st = inet_pton( AF_INET, hostip, &serverAddress.sin_addr );

    if ( st == -1 ) {	// 0 means invalid address, -1 means address error
      throw std::runtime_error( "Socket::Connect( const char *, int ) [inet_pton]" );
    }

    serverAddress.sin_port = htons( port );

    st = connect(this -> idSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if ( -1 == st ) {
      throw std::runtime_error( "Socket::Connect( const char *, int ) [connect]" );
    }
  }

  return 0;
}
 
 
 /**
   * EstablishConnection method
   *   use "connect" Unix system call
   *
   * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
   * @param      char * service: process address, example "http"
   *
  **/
 int VSocket::EstablishConnection( const char *host, const char *service ) {

  if (isdigit(service[0])) {
    int port = atoi(service);
    return EstablishConnection(host, port);
  } 
  
  struct addrinfo hints, *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = this -> type == 's'? SOCK_STREAM : SOCK_DGRAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;          /* Any protocol */

  int st = getaddrinfo( host, service, &hints, &result );
  if (st != 0) {
    throw std::runtime_error("getaddrinfo failed");
  }

  for ( rp = result; rp; rp = rp->ai_next ) {
    st = connect( this -> idSocket, rp->ai_addr, rp->ai_addrlen );
    if ( 0 == st )
      break;
  }

  freeaddrinfo( result );

  return 0;
}

int VSocket::Bind(int port) {
  int opt = 1;
  if (setsockopt(this->idSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    throw std::runtime_error("setsockopt SO_REUSEADDR failed");
  }

  int st = 0;

  if (this->IPv6) {
    sockaddr_in6 host6;
    memset(&host6, 0, sizeof(host6));
    host6.sin6_family = AF_INET6;
    host6.sin6_addr = in6addr_any;
    host6.sin6_port = htons(port);
    st = bind(this->idSocket, (struct sockaddr*)&host6, sizeof(host6));

  } else {
    sockaddr_in host4;
    memset(&host4, 0, sizeof(host4));
    host4.sin_family = AF_INET;
    host4.sin_addr.s_addr = htonl(INADDR_ANY);
    host4.sin_port = htons(port);
    st = bind(this->idSocket, (struct sockaddr*)&host4, sizeof(host4));
  }

  if (st == -1) {
    throw std::runtime_error("bind");
  }

  return st;
}

/**expression must be a modifiable lvalue
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

  int st = 0;
  socklen_t addrLen;

  if(this->IPv6) {
    addrLen = sizeof(struct sockaddr_in6);
  } else {
    addrLen = sizeof(struct sockaddr_in);
  }

  if (this->type == 's') {
    st = send(this->idSocket, buffer, size, 0);
  } else {
    st = sendto(this->idSocket, buffer, size, 0, (struct sockaddr*)addr, addrLen);
  }

  if (st == -1) {
    throw std::runtime_error("sendTo");
  }

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
  int st = 0;
  socklen_t addrLen;

  if(this->IPv6) {
    addrLen = sizeof(struct sockaddr_in6);
  } else {
    addrLen = sizeof(struct sockaddr_in);
  }
  

  if (this->type == 's') {
    st = recv(this->idSocket, buffer, size, 0);
  } else {
    st = recvfrom(this->idSocket, buffer, size, 0, (struct sockaddr*)addr, &addrLen);
  }

  if (st == -1) {
    throw std::runtime_error("recvFrom");
  }

  
  return st;
}

