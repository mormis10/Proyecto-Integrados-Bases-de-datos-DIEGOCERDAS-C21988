#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Socket.h"
#include "VSocket.h"

//Ojito pq este va a ser el puerto de nuestro servidor remoto 
#define PORT 5678  
#define MAXLINE 1024

// Función para simular la respuesta con una imagen ASCII
char* buscar_imagen_ascii(const char *nombre_imagen) {
    printf("Esto es lo que me está llegando %s\n");
    if (strcmp(nombre_imagen, "/imagen_ascii") == 0) {
        return 
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
"";
    }
    return "Imagen no encontrada.";
}

//char* isla_1()

int main() {
    int sockid, haverid;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[MAXLINE];
    int n;

    // Creamos ne
    sockid = socket(AF_INET, SOCK_STREAM, 0);
    if (sockid < 0) {
        perror("Error al crear socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Enlazar el socket
    if (bind(sockid, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al enlazar socket");
        exit(1);
    }

    // Aquí empezamos a escuchar por conexiones para luego hacer el accept
    if (listen(sockid, 5) < 0) {
        perror("Error al escuchar");
        exit(1);
    }

    printf("Servidor remoto escuchando en el puerto %d...\n", PORT);

    while (1) {
        client_len = sizeof(client_addr);
        haverid = accept(sockid, (struct sockaddr*)&client_addr, &client_len);
        if (haverid < 0) {
            perror("Error al aceptar conexión");
            continue;
        }

        // Recibir la solicitud del servidor tenedor
        memset(buffer, 0, sizeof(buffer));
        n = read(haverid, buffer, sizeof(buffer));
        if (n <= 0) {
            perror("Error al leer del cliente");
            close(haverid);
            continue;
        }

        printf("Solicitud recibida: %s\n", buffer);
        char* method = strtok(buffer, " ");
        char* path = strtok(NULL, " "); 
        printf("%s \n",path);


        // Buscar la imagen ASCII
        char* imagen_ascii = buscar_imagen_ascii(path);

        // Enviar la imagen ASCII al servidor tenedor
        write(haverid, imagen_ascii, strlen(imagen_ascii));

        close(haverid);
    }

    close(sockid);
    return 0;
}
