/*
 *  Ejemplo de sockets con IPv4
 *
 */

#include <stdio.h>
#include <string.h>

#include "VSocket.h"
#include "Socket.h"

int main( int argc, char * argv[] ) {
   const char * os = "http://os.ecci.ucr.ac.cr/";
   // cuando es una dirección privada  está el osi, accesible solo dentro de una red interna (esta funciona dentro de la u)
   const char * osi = "10.84.166.62";
   //Esta dirreción es pública y accesible desde cualquier parte del internet
   const char * ose = "163.178.104.62";
   //Esta es la petición http queda todo en el socket
   const char * whale = (char *) "GET /aArt/index.php?disk=Disk-01&fig=whale-1.txt\r\nHTTP/v1.1\r\nhost: redes.ecci\r\n\r\n";

   VSocket * s;	
   char a[512];

   s = new Socket( 's' );
   s->MakeConnection( ose, 80 );
   s->Write( whale );
   s->Read( a, 512 );
   printf( "%s\n", a);
   s->Close();

}

