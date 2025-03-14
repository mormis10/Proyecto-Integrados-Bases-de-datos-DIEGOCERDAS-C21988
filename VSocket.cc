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
#include <cerrno>
#include <cstring>		// memset
#include <cstdio>
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
    this->IPv6 = IPv6;
    this->type = t;

    // tenemos que ver el tipo de dominio, cuando es de tipo IPv6 utilizamos el dominio AF_INET6 y en caso de IPv4 utilizamos AF_INET
    int domain = AF_INET;
   // debería de utilizar más el operador terniario
   int socket_type = SOCK_STREAM;
    
    if(socket_type == -1){
        throw std::runtime_error("VSocket::BuildSocket - Tipo de socket inválido");
    }

    //Ahora utilizo la biblioteca para crear el socket
    // para crear el socket y obtener su id ocupo el dominio y el tipo de socket
    this->idSocket = socket(domain,socket_type,0);

    if(this->idSocket == -1){
        throw std::runtime_error("VSocket::BuildSocket - Error al crear el socket");
    }else{
        printf("Hecho con éxito nachito\n");
    }

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
   if(this->idSocket != -1){
    //busca el id dl socket y lo cierra
    if(close(this->idSocket) == -1){
       throw std::runtime_error("VSocket::Close()" );
    }
    printf("Cierre realizado con éxito\n");
    this->idSocket = -1;
   }else{
     throw std::runtime_error( "VSocket::Close()" );
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
int VSocket::EstablishConnection( const char * hostip, int port ) {
   int st = -1;
   this->port = port;
            struct sockaddr_in  host4;
            memset( (char *) &host4, 0, sizeof( host4 ) );
            host4.sin_family = AF_INET;
            st = inet_pton( AF_INET, hostip, &host4.sin_addr );
            if ( -1 == st ) {
               throw( std::runtime_error( "VSocket::DoConnect, inet_pton" ));
            }
            host4.sin_port = htons( port );
            st = connect( this->idSocket, (sockaddr *) &host4, sizeof( host4 ) );
            if ( -1 == st ) {
                perror("Error en connect");
               throw( std::runtime_error( "VSocket::DoConnect, connect" ));
            }

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::EstablishConnection" );
   }

   return st;

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


   int st = -1;
   struct  addrinfo hints, *res, *p;
   // para esto tenemos que con hints es la estrucutra par definir el tipo de conexión que se desea 
   //res va a tener la lista de direcciones y con p vamos a recorrer las direcciones obtenidas 
   
   // esto ya lo sabemos solo estamos añadiendo ceros dentro de hints para limpiarlo en caso de que hubiera basura
   memset(&hints,0,sizeof hints);
  
  // Asignamos AF_INET6 para indicarle que deseamos solamente conexiones de tipo Ipv6
   hints.ai_family = AF_INET6;
   //Aquí específicamos que queremos un socket TCP

   //busvamos la dirección del servidor
   if(getaddrinfo(host,service,&hints,&res)!= 0){
     throw std::runtime_error("Falló el proceso de getaddinfo\n");
   }

   //Ahora sí vamos a recorrer el arrego de direcciones que tenemos
   // lo recorremos cómo sí fuera una lista enlazada

   for(p = res; p != nullptr; p = p->ai_next){
    //creamos el socket
    // obtenemos el id del socket
    this->idSocket = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
    // si la conexión falla, continue con la siguiente
     if(this->idSocket == -1){
      continue;
     }
     // en caso de que sí fuera exitoso
     // en ese caso sí nos conectamos 

     st = connect(this->idSocket,p->ai_addr,p->ai_addrlen);
     // en caso de que la conexión se haya realizado de manera correcta 
     // dejamos de recorrer el arreglo 
     if(st == 0){
       break;
     }
     // en caso de que algo haya pasado mal, entonces cerramos el socket
     close(this->idSocket);
   }
   // liberamos la memoria dinámica alojada en res
    freeaddrinfo(res);

   
   if(st == -1){
   throw std::runtime_error( "VSocket::EstablishConnection" );
   }

   return st;

}

