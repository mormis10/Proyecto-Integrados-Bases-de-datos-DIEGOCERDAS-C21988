#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>

#include "VSocket.h"
#include "Socket.h"

#define PORT 1234
#define MAXLINE 1024

typedef struct{
    int client_portid;
} client_info;

typedef struct{
int number_of_islands;
std::string* island_ips;
int* ports;
}island_data_t;

typedef struct{
std::string figure_name;
std::string figure;
char* ip_direction;
}thread_data;


void set_island_data(island_data_t*){

}

void connection(){
    // Vamos a dejarlo quemado

}

void start_servers_connection(){
    int sockid, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t thread_id;

    sockid = socket(AF_INET,SOCK_STREAM,0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5678);  // Puerto del servidor remoto
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
}


void* client_handle(void* arg) {
    client_info* data = (client_info*)arg;
    int client_sockid = data->client_portid;
    char buffer[MAXLINE];

    // Leemos la solicitud del cliente
    int n = read(client_sockid, buffer, MAXLINE);

    if (n <= 0) {
        perror("Error al leer del cliente");
        free(data);  // Liberamos la memoria
        return NULL;
    }
    std::cout << "La solicitud se recibió con éxito\n";

    // Ahora nos conectamos al servidor remoto que contiene la imagen
    int remote_id;
    struct sockaddr_in server_remote;

    remote_id = socket(AF_INET, SOCK_STREAM, 0);
    if (remote_id < 0) {
        perror("Error al crear socket remoto");
        free(data);  // Liberamos la memoria
        return NULL;
    }

    server_remote.sin_family = AF_INET;
    server_remote.sin_port = htons(5678);  // Puerto del servidor remoto
    server_remote.sin_addr.s_addr = inet_addr("127.0.0.1");  // IP del servidor remoto

    // Nos conectamos al servidor remoto
    if (connect(remote_id, (struct sockaddr*)&server_remote, sizeof(server_remote)) < 0) {
        perror("Error al conectar con el servidor remoto");
        close(remote_id);
        free(data);  // Liberamos la memoria
        return NULL;
    }

    // Enviamos la solicitud de imagen al servidor remoto
    write(remote_id, buffer, strlen(buffer));

    // Recibimos la imagen ASCII del servidor remoto
    memset(buffer, 0, sizeof(buffer));
    n = read(remote_id, buffer, sizeof(buffer));
    if (n <= 0) {
        perror("Error al recibir respuesta del servidor remoto");
        close(remote_id);
        free(data);  // Liberamos la memoria
        return NULL;
    }

    // Mostramos la imagen ASCII recibida
    printf("Imagen ASCII recibida del servidor remoto:\n%s\n", buffer);

    // Enviamos la imagen ASCII al cliente
    if (write(client_sockid, buffer, strlen(buffer)) < 0) {
        perror("Error al enviar respuesta al cliente");
    }

    // Cerramos los sockets
    close(remote_id);
    close(client_sockid);

    // Liberamos la memoria
    free(data);

    return NULL;
}

int main() {
    int sockid, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t thread_id;

    // Crear socket para el servidor
    sockid = socket(AF_INET, SOCK_STREAM, 0);
    if (sockid < 0) {
        perror("Error al crear socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Enlazar el socket con la dirección
    if (bind(sockid, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al enlazar socket");
        exit(1);
    }

    // Aquí empezamos a verificar que el servidor esté escuchando
    if (listen(sockid, 5) < 0) {
        perror("Error escuchando por solicitudes");
        exit(1);
    }

    // Este es el hilo con el que vamos a trabajar
    while (true) {
        client_len = sizeof(client_addr);
        int client_id = accept(sockid, (struct sockaddr*)&client_addr, &client_len);
        if (client_id < 0) {
            perror("Error al aceptar conexión");
            continue;
        }

        // Asignamos información del cliente y creamos el hilo
        client_info *data = (client_info*)malloc(sizeof(client_info));
        data->client_portid = client_id;

        // Crear el hilo para manejar la solicitud del cliente
        if (pthread_create(&thread_id, NULL, client_handle, (void*)data) != 0) {
            perror("Error al crear el hilo");
            free(data);  // Liberamos la memoria si hay un error
            close(client_id);
            continue;
        }

        // Usamos pthread_detach para que el hilo se libere automáticamente
        pthread_detach(thread_id);
    }

    // Cerramos el socket principal
    close(sockid);
    return 0;
}
