#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>


void server_connection_prove(std::string* art, int* sockets){
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
    sockets[i] = id;
    char* buffer = new char[200];
    read(id,buffer,200);
    std::string str(buffer);
    art[i] = str;
    delete[] buffer;
    }

}

void user_menu(std::string* art, int* sockets){
    bool menu = true;
    int option = 0;
    std::string image_option = "";
    int flag = 0;
    while(menu){
        flag = 0;
        std::cout<< "Bienvenido al menú de las imagenes de los servers\n";
        for(int i = 0; i<6; i++){
            std::cout<<"Imágenes disponibles\n";
            std::cout<<art[i] <<"\n";
        }
        std::cout<<"Ingrese 1 si desea obtener la imagen o 2 para salir del menú\n";
        std::cin>>option;
        switch(option){
            case 1:
            std::cout<<"Ingrese el nombre de la figura que desea obtener\n";
            std::cin>>image_option;
            for(int i = 0; i<6; i++){
                if(art[i] == image_option){
                    flag = 1;
                    write(sockets[i],image_option.c_str(),image_option.size());
                    char* buffer = new char[200];
                    read(sockets[i],buffer,200);
                    std::cout<<"Imagen ASCII Resultante:\n";
                    std::cout<<buffer <<"\n";
                    delete[] buffer;
                }
            }
            if(flag == 0){
                std::cout<<"Imagen no encontrada\n";
            }
            
            break;
            case 2:
            std::cout<<"Hasta luego, muchas gracias!!\n";
            menu = false;
            break;
        }
    }
    
}

int main(){
    std::string figures_name[6];
    int servers_sockets[6];
    server_connection_prove(figures_name,servers_sockets);
    user_menu(figures_name,servers_sockets);
    for(int i = 0; i<6; i++){
        close(servers_sockets[i]);
    }
    return 0;
}