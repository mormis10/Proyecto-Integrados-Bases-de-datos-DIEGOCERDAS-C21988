#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include "Socket.h"

void server_connection_prove(char** art, int* socket_fd, char** ip_out) {
    int udp_id = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_id < 0) {
        perror("Error al crear el socket UDP");
        return;
    }

    struct sockaddr_in udp_mess;
    memset(&udp_mess, 0, sizeof(udp_mess));
    udp_mess.sin_family = AF_INET;
    udp_mess.sin_port = htons(8081);  // Solo un puerto
    if (inet_pton(AF_INET, "127.0.0.1", &udp_mess.sin_addr) <= 0) {
        perror("Error en inet_pton");
        close(udp_id);
        return;
    }

    const char* request_udp = "GET /servers";
    if (sendto(udp_id, request_udp, strlen(request_udp), 0,
               (struct sockaddr*)&udp_mess, sizeof(udp_mess)) < 0) {
        perror("Error en sendto");
        close(udp_id);
        return;
    }

    // Recibir respuesta UDP
    int udp_recieve = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(udp_recieve, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        close(udp_recieve);
        return;
    }

    struct sockaddr_in other;
    socklen_t addr_len = sizeof(other);
    char data[200] = {0};
    ssize_t st = recvfrom(udp_recieve, data, 200, 0,
                          (struct sockaddr*)&other, &addr_len);

    if (st < 0) {
        perror("Error al recibir respuesta UDP");
        close(udp_recieve);
        return;
    }

    data[st] = '\0';
    std::string pivot(data);
    size_t space_pos = pivot.find(' ');
    std::string ip = pivot.substr(0, space_pos);
    std::string name = pivot.substr(space_pos + 1);

    *art = strdup(name.c_str());
    *ip_out = strdup(ip.c_str());

    std::cout << "Recibí información:\n";
    std::cout << "IP: " << *ip_out << "\n";
    std::cout << "Figura: " << *art << "\n";

    close(udp_id);
    close(udp_recieve);

    // Conectar vía TCP
    struct sockaddr_in server_addr_tcp;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr_tcp, 0, sizeof(server_addr_tcp));
    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_port = htons(8081);
    server_addr_tcp.sin_addr.s_addr = inet_addr("127.0.0.1");

    usleep(500000);  

    if (connect(sock, (struct sockaddr*)&server_addr_tcp, sizeof(server_addr_tcp)) < 0) {
        perror("Error al conectar con el servidor TCP");
        close(sock);
        return;
    }

    *socket_fd = sock;
}

void user_menu(char* art, int socket_fd) {
    int sockid = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr_tcp, client_addr;
    socklen_t client_len = sizeof(client_addr);
    memset(&server_addr_tcp, 0, sizeof(server_addr_tcp));
    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_addr.s_addr = INADDR_ANY;
    server_addr_tcp.sin_port = htons(8080);

    if (bind(sockid, (struct sockaddr*)&server_addr_tcp, sizeof(server_addr_tcp)) < 0) {
        perror("Error al enlazar socket");
        exit(1);
    }

    if (listen(sockid, 5) < 0) {
        perror("Error escuchando por solicitudes");
        exit(1);
    }

    int client_id = accept(sockid, (struct sockaddr*)&client_addr, &client_len);
    if (client_id < 0) {
        perror("Error al aceptar conexión");
    }

    std::string concat = "Imágenes disponibles: " + std::string(art);
    char* offer = (char*) concat.c_str();
    write(client_id,offer,strlen(offer));
    char buffer[512];
    read(client_id,buffer,512);
    char* image_option = strdup(buffer);
    if(strcmp(art, image_option) == 0){
    write(socket_fd,image_option,strlen(image_option));
    char bresponse[512];
    read(socket_fd,bresponse,512);
    char* cresponse = strdup(bresponse);
    write(client_id,cresponse,strlen(cresponse));
    }else{
        char* error_response = "Ocurrió un error imágen no encontrada";
        write(client_id,error_response,strlen(error_response));
    }

    std::cout << "Hasta luego, muchas gracias!\n";
}

int main() {
    char* figure_name = nullptr;
    int server_socket = -1;
    char* ip = nullptr;

    server_connection_prove(&figure_name, &server_socket, &ip);
    if (server_socket != -1 && figure_name != nullptr) {
        user_menu(figure_name, server_socket);
        close(server_socket);
        free(figure_name);
        free(ip);
    }

    return 0;
}
