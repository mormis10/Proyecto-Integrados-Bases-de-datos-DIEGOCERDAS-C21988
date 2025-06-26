#include <sys/socket.h>  // sockaddr_in
#include <arpa/inet.h>   // ntohs
#include <unistd.h>      // write, read
#include <cstring>
#include <stdexcept>
#include <stdio.h>       // printf
#include <iostream>      // cerr, cout
#include "Socket.h"      // Derived class


Socket::Socket(char t, bool IPv6) {
  this->BuildSocket(t, IPv6);  // Call base class constructor
}

Socket::Socket(int fd) {
  this->IDSocket = fd;
  this->type = 's';  // Assume TCP

  // Detect if it is IPv6 (same as in BuildSocket)
  struct sockaddr_storage ss;
  socklen_t len = sizeof(ss);
  if (getsockname(fd, (struct sockaddr*)&ss, &len) == 0) {
    this->IPv6 = (ss.ss_family == AF_INET6);
  } else {
    this->IPv6 = false;
  }
}

/**
 * Class destructor
 *
 * @param int ID: socket descriptor
 */
Socket::~Socket() {
  this->Close();
}

size_t Socket::Read(void *buffer, size_t size) {
  // read() is a function that attempts to read up to <size> bytes from the socket (IDSocket) and stores them in buffer:
  ssize_t bytesRead = read(this->IDSocket, buffer, size);  // Returns the number of bytes read on success

  if (bytesRead == -1) {
    throw std::runtime_error("Socket::Read: Error reading from socket\n");
  } else if (bytesRead == 0) {
    //std::cerr << "Socket::Read: Connection closed by peer\n";
  } else {
    //std::cout << "Socket::Read: Read from socket successfully\n";
  }

  // read() returns a ssize_t (can be negative in case of error), but our function returns a size_t (unsigned value)
  return static_cast<size_t>(bytesRead);  // Cast the value to return the correct value type
}

size_t Socket::Write(const void *buffer, size_t size) {
  // write() is a function that attempts to send (write) <size> bytes from buffer through the socket (IDSocket):
  ssize_t bytesWritten = write(this->IDSocket, buffer, size);  // Returns the number of bytes sent on success

  if (bytesWritten == -1) {
    throw std::runtime_error("Socket::Write: Error writing to socket\n");
  } else {
  }

  return static_cast<size_t>(bytesWritten);  // Cast the value to return the correct value type
}

size_t Socket::Write(const char *text) {
  return this->Write(static_cast<const void *>(text), strlen(text));
}

VSocket* Socket::AcceptConnection() {
  if (this->type != 's') {
    throw std::runtime_error("Only TCP (stream) sockets can accept connections");
  }

  struct sockaddr_storage client_addr;  // Compatible with IPv4 and IPv6
  socklen_t addr_len = sizeof(client_addr);

  int client_fd = accept(this->IDSocket, (struct sockaddr*)&client_addr, &addr_len);
  if (client_fd == -1) {
    throw std::runtime_error("Accept failed: " + std::string(strerror(errno)));
  }

  // Create a new socket with the same address family
  Socket* client = new Socket(client_fd);
  client->IPv6 = (client_addr.ss_family == AF_INET6);
  return client;
}
