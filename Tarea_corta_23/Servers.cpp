#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#include "File_System.hpp"

typedef struct {
    char* figure_name;
    char* figure;
    char* ip_direction;
    int my_port;
    int server_port;
    int island_id;
}server_data;

void start_servers_connection(server_data* data) {
    int sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(data->my_port);

    if (bind(sock_udp, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        close(sock_udp);
        return;
    }

    struct sockaddr_in other;
    socklen_t addr_len = sizeof(other);
    char buffer_udp[201];
    ssize_t st = recvfrom(sock_udp, buffer_udp, 200, 0,
                          (struct sockaddr*)&other, &addr_len);
    if (st < 0) {
        perror("Error en recvfrom");
        close(sock_udp);
        return;
    }
    buffer_udp[st] = '\0';
    std::cout << "Mensaje recibido: " << buffer_udp << "\n";

    int sock_udp_send = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_udp_send < 0) {
        perror("Error al crear socket de envío");
        close(sock_udp);
        return;
    }

    struct sockaddr_in udp_mess;
    memset(&udp_mess, 0, sizeof(udp_mess));
    udp_mess.sin_port = htons(8080);
    udp_mess.sin_family = AF_INET;

    if (inet_pton(AF_INET, "127.0.0.1", &udp_mess.sin_addr) <= 0) {
        perror("Error en inet_pton");
        close(sock_udp);
        close(sock_udp_send);
        return;
    }

    std::string result = std::string("127.0.0.1 ") + std::string(data->figure_name);
    const char* message = result.c_str();
    ssize_t sent_bytes = sendto(sock_udp_send, message, strlen(message), 0,
                                (const struct sockaddr*)&udp_mess,
                                sizeof(udp_mess));

    if (sent_bytes < 0) {
        perror("Error en sendto");
    } else {
        std::cout << "Enviados " << sent_bytes << " bytes a 127.0.0.1:8080\n";
    }

    // TCP Parte
    int sockid = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr_tcp, client_addr;
    socklen_t client_len = sizeof(client_addr);
    memset(&server_addr_tcp, 0, sizeof(server_addr_tcp));
    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_addr.s_addr = INADDR_ANY;
    server_addr_tcp.sin_port = htons(data->my_port);

    if (bind(sockid, (struct sockaddr*)&server_addr_tcp, sizeof(server_addr_tcp)) < 0) {
        perror("Error al enlazar socket");
        exit(1);
    }

    if (listen(sockid, 5) < 0) {
        perror("Error escuchando por solicitudes");
        exit(1);
    }

    std::cout << "Servidor (Isla) #" << data->island_id << " escuchando...\n";

    int client_id = accept(sockid, (struct sockaddr*)&client_addr, &client_len);
    if (client_id < 0) {
        perror("Error al aceptar conexión");
    }

    std::cout << "Servidor (Isla) #" << data->island_id << " aceptó conexión\n";

    char buffer2[200] = {0};
    int bytes_read2 = read(client_id, buffer2, 200);
    buffer2[bytes_read2] = '\0';
    printf("Recibo esta solicitud: %s \n", buffer2);

    const char* image2 =
        "   ******       ******\n"
        "  **      **   **      **\n"
        " **         ** **         **\n"
        "**           ***           **\n"
        "**            *            **\n"
        "**                         **\n"
        " **                       **\n"
        "  **                     **\n"
        "   **                   **\n"
        "     **               **\n"
        "       **           **\n"
        "         **       **\n"
        "           **   **\n"
        "             **\n";


    const char* fresponse = (const char*) data->figure;
    write(client_id, fresponse, strlen(fresponse));
    close(sock_udp);
    close(sock_udp_send);
    return;
}

void add_figures_server(std::string name, std::string figure, FileSystem* f){
        f->create_file(name, figure);
}

void set_server_data(server_data* data, FileSystem* f){
    data->figure_name = "Flecha";
    data->figure = f->get_content("Flecha");
    data->ip_direction = (char*)"127.0.0.1";
    data->server_port = 8080;
    data->my_port = 8081;
    data->island_id = 1;

}

int main() {
    server_data data;
    FileSystem f("disk.mfs");
    f.format();
    char* figure_name = "Flecha";
    char* figure_content[6] = {
        "   ^   \n"
        "  / \\  \n"
        " /___\\ \n"
        "   |   \n"
        "   |   ",
        " [o_o] \n"
        "/|_|\\ \n"
        " / \\  ",
        "  /\\  \n"
        " /__\\ \n"
        " |==| \n"
        "/____\\",
        " /\\_/\\ \n"
        "( o.o )\n"
        " > ^ < ",
        " .-.  \n"
        "(o o) \n"
        "| O | \n"
        "'---'",
        "  /\\  \n"
        " /__\\ \n"
        " |==| \n"
        "/____\\"
    };
    std::string prove = (std::string) figure_content[0];

    add_figures_server("Flecha", prove,&f);

    int ports[6] = {8001, 8002, 8003, 8004, 8005, 8006};
    char* ip = (char*)"127.0.0.1";

    set_server_data(&data,&f);
    start_servers_connection(&data);

    return 0;
}
