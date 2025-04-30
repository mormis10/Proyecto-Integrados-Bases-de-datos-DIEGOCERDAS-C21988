/**
 *  Universidad de Costa Rica
 *  ECCI
 *  CI0123 Proyecto integrador de redes y sistemas operativos
 *  2025-i
 *  Grupos: 1 y 3
 *
 *  ******   Socket class implementation
 *
 * (Fedora version)
 *
 **/

#include <arpa/inet.h> // ntohs
#include <cstring>
#include <stdexcept>    // esto es para la biblio de runtime error
#include <stdio.h>      // printf
#include <sys/socket.h> // sockaddr_in
#include <unistd.h>     // write, read

#include "Socket.h" // Derived class

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <stdexcept>

/**
 *  Class constructor
 *     use Unix socket system call
 *
 *  @param     char t: socket type to define
 *     's' for stream
 *     'd' for datagram
 *  @param     bool ipv6: if we need a IPv6 socket
 *
 **/
Socket::Socket(char t, bool IPv6) {

  this->BuildSocket(t, IPv6); // Call base class constructor
}

Socket::Socket(int id, bool ipv6){
  printf("Entra aquí\n");
  this->BuildSocket(id,ipv6);
}

/**
 *  Class destructor
 *
 *  @param     int id: socket descriptor
 *
 **/
Socket::~Socket() {}

/**
 * MakeConnection method
 *   use "EstablishConnection" in base class
 *
 * @param      char * host: host address in dot notation, example "10.1.166.62"
 * @param      int port: process address, example 80
 *
 **/
int Socket::MakeConnection(const char *hostip, int port) {

  return this->EstablishConnection(hostip, port);
}

/**
 * MakeConnection method
 *   use "EstablishConnection" in base class
 *
 * @param      char * host: host address in dns notation, example
 *"os.ecci.ucr.ac.cr"
 * @param      char * service: process address, example "http"
 *
 **/
int Socket::MakeConnection(const char *host, const char *service) {

  return this->EstablishConnection(host, service);
}

/**
 * Read method
 *   use "read" Unix system call (man 3 read)
 *
 * @param      void * buffer: buffer to store data read from socket
 * @param      int size: buffer capacity, read will stop if buffer is full
 *
 **/
size_t Socket::Read(void *buffer, size_t size) {
  // Aquí tenemos que utilizar la función de read
  // para los parámetros ocupamos el id del socket-el buffer y el tamaño del
  // buffer
  ssize_t read_bytes = read(this->idSocket, buffer, size);

  if (read_bytes == -1) {
    throw std::runtime_error("Ocurrió un error obteniendo la cantidad de bytes "
                             "que debemos de leer\n");
  }

  // Ahora toca castearlo para que vuelva a ser un entero sin signo
  static_cast<size_t>(read_bytes);

  return read_bytes;
}

/**
 * Write method
 *   use "write" Unix system call (man 3 write)
 *
 * @param      void * buffer: buffer to store data write to socket
 * @param      size_t size: buffer capacity, number of bytes to write
 *
 **/
size_t Socket::Write(const void *buffer, size_t size) {

  ssize_t written_bytes = write(this->idSocket, buffer, size);
  if (written_bytes == -1) {
    throw std::runtime_error("Ocurrió un error durante la lectura\n");
  }
  // Aquí toca volver a hacer el casteo nacho
  static_cast<size_t>(written_bytes);
  return written_bytes;
}

/**
 * Write method
 *   use "write" Unix system call (man 3 write)
 *
 * @param      char * text: text to write to socket
 *
 **/
size_t Socket::Write(const char *text) {
  return Write(static_cast<const void *>(text), strlen(text));
}

VSocket * Socket::AcceptConnection(){
   int id;
   VSocket * peer;
   printf("LLega aquí\n");

   id = this->WaitForConnection();

   peer = new Socket(id,false);

   printf("Pasa de aquí\n");

   return peer;
}

