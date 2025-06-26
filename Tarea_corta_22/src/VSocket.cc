#include <sys/socket.h>
#include <arpa/inet.h>  // ntohs, htons
#include <stdexcept>    // runtime_error
#include <cstring>      // memset
#include <netdb.h>      // getaddrinfo, freeaddrinfo
#include <unistd.h>     // close
#include <iostream>     // cout
#include "VSocket.h"
#include "Socket.h"

/**
 * Class creator (constructor)
 * Use Unix socket system call
 *
 * @param char t: socket type to define
 *   's' for stream
 *   'd' for datagram
 * @param bool IPv6: if we need a IPv6 socket
 */
void VSocket::BuildSocket(char t, bool IPv6) {
  this->type = t;  // Assign the socket type (s/d)
  this->IPv6 = IPv6;  // Define if it is IPv6 (true/false)

  // Sockets need to be created with: int socket(int domain, int type, int protocol)

  // The domain defines if the socket is IPv4 or IPv6:
  int domain = IPv6 ? AF_INET6 : AF_INET;  // We use the IPv6 boolean indicator

  // We already know the type of the socket, so we assing the corresponding value:
  int socketType = (t == 's') ? SOCK_STREAM : SOCK_DGRAM;

  // We leave the protocol = 0, system picks the right protocol:
  this->IDSocket = socket(domain, socketType, 0);  // Creates the socket, returning its ID value

  if (this->IDSocket == -1) {
    throw std::runtime_error("VSocket::BuildSocket: Failed to create socket\n");
  } else {
    //std::cout << "VSocket::BuildSocket: Socket created successfully\n";
  }
}

void VSocket::BuildSocket(int fd) {
  this->IDSocket = fd;
  this->type = 's';

  // Automatic IPv6 detection
  struct sockaddr_storage ss;
  socklen_t len = sizeof(ss);
  if (getsockname(fd, (struct sockaddr*)&ss, &len) == 0) {
    this->IPv6 = (ss.ss_family == AF_INET6);
  } else {
    this->IPv6 = false;  // Defaults to IPv4 if it cannot be detected
  }
}

// Class destructor
VSocket::~VSocket() {
  this->Close();
}

void VSocket::Close() {
  if (this->IDSocket != -1) {  // Verify if the socket is open
    if (close(this->IDSocket) == -1) {  // Tries to close the socket (returns 0 if successful, -1 if not)
      throw std::runtime_error("VSocket::Close: Failed to close socket\n");
    } else {
      //std::cout << "VSocket::Close: Socket closed successfully\n";
    }
    this->IDSocket = -1;  // Mark the socket as closed to avoid errors
  }
}

int VSocket::EstablishConnection(const char* address, const char* portOrService) {
  if (this->IDSocket == -1) {
    throw std::runtime_error("Socket not initialized");
  }

  struct addrinfo hints, *result, *rp;
  memset(&hints, 0, sizeof(hints));

  // Configuration according to the socket type:
  hints.ai_family = this->IPv6 ? AF_INET6 : AF_UNSPEC;  // AF_UNSPEC allows IPv4 and IPv6
  hints.ai_socktype = (this->type == 's') ? SOCK_STREAM : SOCK_DGRAM;
  hints.ai_flags = AI_ADDRCONFIG;  // Only returns addresses compatible with the system

  // 1: Address and service resolution
  int err = getaddrinfo(address, portOrService, &hints, &result);
  if (err != 0) {
    throw std::runtime_error(gai_strerror(err));
  }

  // 2: Try connecting to every possible address
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if (connect(this->IDSocket, rp->ai_addr, rp->ai_addrlen) == 0) {
      freeaddrinfo(result);
      return EXIT_SUCCESS;
    }
  }

  // If we get here, all connections failed
  freeaddrinfo(result);
  throw std::runtime_error("Could not connect to any address");
}

int VSocket::EstablishConnection(const char* address, int port) {
  return EstablishConnection(address, std::to_string(port).c_str());
}

int VSocket::Bind(int port) {
  int yes = 1;
  if (setsockopt(this->IDSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    throw std::runtime_error("Setsockopt failed: " + std::string(strerror(errno)));
  }

  if (this->IPv6) {  // If using an IPv6 socket
    struct sockaddr_in6 addr6;
    memset(&addr6, 0, sizeof(addr6));
    addr6.sin6_family = AF_INET6;
    addr6.sin6_port = htons(port);
    addr6.sin6_addr = in6addr_any;

    // Option to allow IPv4 and IPv6 on the same socket
    int no = 0;
    setsockopt(this->IDSocket, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no));

    if (bind(this->IDSocket, (struct sockaddr*)&addr6, sizeof(addr6)) == -1) {
      throw std::runtime_error("Error binding IPv6 socket: " + std::string(strerror(errno)));
    }
  } else {  // If using an IPv4 socket
    struct sockaddr_in addr4;
    memset(&addr4, 0, sizeof(addr4));  // Clear the structure
    addr4.sin_family = AF_INET;  // IPv4 address family
    addr4.sin_port = htons(port);  // Set port (convert to network byte order)
    addr4.sin_addr.s_addr = INADDR_ANY;  // Bind to any available IPv4 interface

    // Bind the socket to the specified IPv4 address and port
    if (bind(this->IDSocket, (struct sockaddr*) &addr4, sizeof(addr4)) == -1) {
      throw std::runtime_error("VSocket::Bind: Error binding IPv4 socket");  // Throw exception
    }
  }
  this->port = port;
  return 0;
}

VSocket *VSocket::CreateVSocket(int fd) {
  return new Socket(fd);
}

int VSocket::Listen(int backlog) {
  if (listen(this->IDSocket, backlog) == -1) {
    throw std::runtime_error("VSocket::Listen: Failed to listen on socket");
  }
  return 0;  // Success
}

int VSocket::Shutdown(int how) {
  if (shutdown(this->IDSocket, how) == -1) {
    throw std::runtime_error("VSocket::Shutdown: Failed to shutdown socket");
  }
  return 0;  // Success
}

int VSocket::MarkPassive(int backlog) {
  return this->Listen(backlog);
}
