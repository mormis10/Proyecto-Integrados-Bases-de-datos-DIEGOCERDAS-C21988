#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>

#include "Socket.h"

int main(){
    Socket client('s',false);
    client.EstablishConnection("127.0.0.1",8080);
    char buffer[250];
    char* cliente = "Holi soy un cliente";
    client.Read(buffer,250);
    printf("Información recibida del servidor\n");
    printf("%s \n",buffer);
    char* solicitud = "Flecha";
    client.Write(solicitud,strlen(solicitud));
    char rbuffer[512];
    client.Read(rbuffer,512);
    printf("Respuesta: %s\n",rbuffer);
    client.Close();
    return 0;
}
