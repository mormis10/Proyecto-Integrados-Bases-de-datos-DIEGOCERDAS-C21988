/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  ****** SSLSocket example 
  *
  * (Fedora version)
  *
 **/

#include <cstdlib>
#include <cstring>	// strlen
#include <cstdio>
#include <signal.h>
#include "SSLSocket.h"

/**
 *
 **/
int main(int cuantos, char * argumentos[] ) {
   SSLSocket * client;
   char userName[16] = { 0 };
   char password[16] = { 0 };
   const char * requestMessage = "\n<Body>\n\
\t<UserName>%s</UserName>\n\
\t<Password>%s</Password>\n\
</Body>\n";
signal(SIGPIPE, SIG_IGN);
   char buf[1024];
   char clientRequest[ 1024 ] = { 0 };
   int bytes;
   char *hostname, *portnum;
   
   client = new SSLSocket( 's',false );
   if ( cuantos != 3 ) {
      printf("usage: %s <hostname> <portnum>\n", argumentos[0] );
      exit(0);
   }
   hostname = argumentos[ 1 ];
   portnum = argumentos[ 2 ];
   client->InitContext();
   client->EstablishConnection( hostname, atoi( portnum ) );
   client->SSLInit();
   printf("Pasa de aquí 2\n");
   printf( "Enter the User Name : " );
   scanf( "%s", userName );
   printf( "\nEnter the Password : " );
   scanf( "%s", password );
   sprintf( clientRequest, requestMessage, userName, password );	// construct reply
   printf( "\n\nConnected with %s encryption\n", client->GetCipher() );
   client->ShowCerts();		// display any certs
   client->Write( clientRequest, strlen( clientRequest ) );		// encrypt & send message
   bytes = client->Read( buf, sizeof( buf ) );			// get reply & decrypt
   buf[ bytes ] = 0;
   printf("Received: \"%s\"\n", buf);

   return 0;

}
