#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>


void server_connection_prove(){

    int id;
    int ports[6] = {8001,8002,8003,8004,8005,8006};
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    id = socket(AF_INET,SOCK_STREAM,0);
    //Simulamos un brodcast donde hacemos connect con todas las islas
    char* solicitud = "GET/FIGURES";
    for(int i = 0; i<6; i++){
     id = socket(AF_INET,SOCK_STREAM,0);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(ports[i]);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
     // Nos conectamos al servidor remoto
    if (connect(id, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar con el servidor remoto");
        close(id);
          // Liberamos la memoria
        return;
    }
    
    // Enviamos la solicitud de imagen al servidor remoto
    write(id, solicitud, strlen(solicitud));

    }

}

int main(){
    server_connection_prove();
    return 0;
}