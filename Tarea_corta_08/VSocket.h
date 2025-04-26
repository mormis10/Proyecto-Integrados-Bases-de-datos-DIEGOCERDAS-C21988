
/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  ****** VSocket base class interface
  *
  * (Fedora version)
  *
 **/
#include <cstddef> 

#ifndef VSocket_h
#define VSocket_h
 
class VSocket {
   public:
       void BuildSocket( char, bool = false );
       void BuildSocket(int id,bool ipv6);
      ~VSocket();

      void Close();
      int EstablishConnection( const char *, int );
      int EstablishConnection( const char *, const char * );
      int EstablishConnection( const char *, int ,bool);
      virtual int MakeConnection( const char *, int ) = 0;
      virtual int MakeConnection( const char *, const char * ) = 0;

      virtual size_t Read( void *, size_t ) = 0;
      virtual size_t Write( const void *, size_t ) = 0;
      virtual size_t Write( const char * ) = 0;
      int Bind(int port, bool Ipv6);
      size_t sendTo( const void *, size_t, void * );
      size_t recvFrom( void *, size_t, void * );
      int get_idSocket();
      int MarkPassive( int );		// Mark a socket passive: will be used to accept connections
      int WaitForConnection( void );	// Wait for a peer connection
      virtual VSocket * AcceptConnection() = 0;
      int Shutdown( int );

   protected:
      int idSocket;   // Socket identifier
      bool IPv6;      // Is IPv6 socket?
      int port;       // Socket associated port
      char type;      // Socket type (datagram, stream, etc.)
        
};

#endif // VSocket_h
