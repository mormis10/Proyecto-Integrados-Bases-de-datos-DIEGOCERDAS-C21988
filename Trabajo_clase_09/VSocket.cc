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
  int socket_type;
   if(this->type == 's'){
     socket_type = SOCK_STREAM;
   }else{
    socket_type = SOCK_DGRAM;
   }

   if(!IPv6){
  // tenemos que ver el tipo de dominio, cuando es de tipo IPv6 utilizamos el
  // dominio AF_INET6 y en caso de IPv4 utilizamos AF_INET
  
  // debería de utilizar más el operador terniario
  domain = AF_INET;

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
}else{
  domain = AF_INET6;
  this->idSocket = socket(domain, socket_type, 0);

  if (this->idSocket == -1) {
    throw std::runtime_error("VSocket::BuildSocket - Error al crear el socket");
  } else {
    printf("Hecho con éxito nachito ipv6\n");
  }
}
printf("Intentando conectar al servidor...\n");
  return;
}
void VSocket::BuildSocket(int id,bool ipv6){
  this->idSocket = id;
  this->IPv6 = false;  // o true si estás usando IPv6
  this->type = 's'; 
  this->IPv6 = ipv6;
  printf("Entra aquí\n");
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
  printf("Entra");
  int st = -1;
  this->port = port;
  struct sockaddr_in host4;

  memset((char *)&host4, 0, sizeof(host4));
  host4.sin_family = AF_INET;
  // Verifica si la dirección IP es válida
  st = inet_pton(AF_INET, hostip, &host4.sin_addr);
  if (st <= 0) {  // inet_pton regresa 0 si la IP no es válida
    if (st == 0) {
      throw std::runtime_error("VSocket::DoConnect, IP inválida");
    }
    throw std::runtime_error("VSocket::DoConnect, inet_pton");
  }

  // Asigna el número de puerto a la estructura
  host4.sin_port = htons(port);

  // Realiza la conexión
  st = connect(this->idSocket, (sockaddr *)&host4, sizeof(host4));
  if (st == -1) {
    perror("Error en connect");
    throw std::runtime_error("VSocket::DoConnect, connect");
  }

  printf("Conexión exitosa\n");
  return st;
}
int VSocket::EstablishConnection(const char *host, int port, bool prove) {
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
  status = getaddrinfo(host,"1234", &hints, &res);
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
  printf("Entra");
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
int VSocket::Bind(int port, bool Ipv6) {
  //Este método se encarga de de asignar una dirección IP y un número de puerto al socket
  //sin este bind el sistema operativo no sabe qué puerto escuchar, le dice al sistema operativo,
  //cualquier paquete que pase por aquí me lo das a mi
  int st = -1;
  if(!Ipv6){
  struct sockaddr_in server;
  memset(&server, 0,sizeof(server));
  server.sin_family = AF_INET;
  st = -1;
  //vamos a escuchar cualquier interfaz de red disponible
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);

   st = bind(this->idSocket,(const struct sockaddr *)&server,sizeof(server));
   if(st<0){
    throw std::runtime_error("Ocurrió un error con el bind\n");
   }
  }else{
    struct sockaddr_in6 host6;
    memset(&host6,0,sizeof(host6));
    host6.sin6_family = AF_INET6;
    st = -1;
    host6.sin6_addr = in6addr_any;
    host6.sin6_port = htons(port);
    st = bind(this->idSocket,(const struct sockaddr *)&host6,sizeof(host6));
   if(st<0){
    throw std::runtime_error("Ocurrió un error con el bind\n");
   }
    
  }

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
   int st = sendto(this->idSocket, buffer, size, 0, (const struct sockaddr *)addr, sizeof(struct sockaddr_in));
    if (st < 0) perror("sendto");
    return (st >= 0) ? static_cast<size_t>(st) : 0;
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
     socklen_t addr_len = sizeof(struct sockaddr_in);
    int st = recvfrom(this->idSocket, buffer, size, 0, (struct sockaddr *)addr, &addr_len);
    if (st < 0) perror("recvfrom");
    return (st >= 0) ? static_cast<size_t>(st) : 0;
}

/**
  * MarkPassive method
  *    use "listen" Unix system call (man listen) (server mode)
  *
  * @param      int backlog: defines the maximum length to which the queue of pending connections for this socket may grow
  *
  *  Establish socket queue length
  *
 **/
int VSocket::MarkPassive( int backlog ) {
   int st = -1;
   st = this->idSocket;

    if (listen(st, backlog) < 0) {
        perror("Error escuchando por solicitudes");
        exit(1);
    }
   return st;
}

/**
  * WaitForConnection method
  *    use "accept" Unix system call (man 3 accept) (server mode)
  *
  *
  *  Waits for a peer connections, return a sockfd of the connecting peer
  *
 **/
int VSocket::WaitForConnection( void ) {
  // Tenemos que llamar a nuestro propio bind, con el puerto en el que vamos a estudiar
   int st = -1;
   struct sockaddr_in client_add;
   socklen_t client_size = sizeof(client_add);
   st = accept(this->idSocket,(struct sockaddr*)&client_add,&client_size);
   return st;
}


/**
  * Shutdown method
  *    use "shutdown" Unix system call (man 3 shutdown) (server mode)
  *
  *
  *  cause all or part of a full-duplex connection on the socket associated with the file descriptor socket to be shut down
  *
 **/
int VSocket::Shutdown( int mode ) {
   int st = -1;

   throw std::runtime_error( "VSocket::Shutdown" );

   return st;

}