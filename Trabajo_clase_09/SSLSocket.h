
/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  *   SSL Socket class interface
  *
  * (Fedora version)
  *
 **/

#ifndef SSLSocket_h
#define SSLSocket_h

#include "VSocket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


class SSLSocket : public VSocket{

   public:
      SSLSocket( char* hostip,int port, bool IPv6 = false );				// Not possible to create with UDP, client constructor
      SSLSocket( char *, char *, bool = false );		// For server connections
      SSLSocket( int );
      SSLSocket(char type, bool IPV6);
      SSLSocket(int, bool);
      ~SSLSocket();
      int Connect( const char *, int );
      int Connect( const char *, const char * );
      const char * GetCipher();
      size_t Read( void *, size_t );
      size_t Write( const void *, size_t );
      size_t Write( const char * );
   public:
      void Init( bool = false );		// Defaults to create a client context, true if server context needed
      void InitContext();
      void LoadCertificates( const char *, const char * );
      void ShowCerts();
      void InitServerContext();
      void InitServer( const char * certFileName, const char * keyFileName);
      void Listen(int);
      SSLSocket* AcceptConnection();
      void Accept();
      void CopyContext(SSLSocket* server);
      void SSLInit();
// Instance variables      
      SSL_CTX* SSLContext;				// SSL context
      SSL* SSLStruct;					// SSL BIO (Basic Input/Output)

};

#endif

