#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>


void server_connection_prove(char* art[], int* sockets,char* ips[]){
    int id;
    int ports[6] = {8001,8002,8003,8004,8005,8006};
    for(int i = 0; i<6; i++){
    int udp_id = socket(AF_INET, SOCK_DGRAM, 0);
if (udp_id < 0) {
    perror("Error al crear el socket UDP");
    return;
}

struct sockaddr_in udp_mess;
memset(&udp_mess, 0, sizeof(udp_mess)); 

udp_mess.sin_port = htons(ports[i]);         
udp_mess.sin_family = AF_INET;

if (inet_pton(AF_INET, "127.0.0.1", &udp_mess.sin_addr) <= 0) {
    perror("Error en inet_pton");
    close(udp_id);
    return;
}

// Mensaje a enviar (sin el null terminator, ya que es UDP)
const char *request_udp = "GET /servers";
ssize_t bytes_sent = sendto(udp_id, request_udp, strlen(request_udp), 0, 
                           (struct sockaddr*)&udp_mess, sizeof(udp_mess));

if (bytes_sent < 0) {
    perror("Error en sendto");
    close(udp_id);
    return;
}

//close(udp_id);
    }
    //Vamos a hacer unas pruebas
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
for(int i = 0; i<6; i++){
 struct sockaddr_in other;
 socklen_t addr_len = sizeof(other);
 char data[200] = {0};

 //Lo pongo a hacer bind

 ssize_t st = recvfrom(udp_recieve, data, 200, 0, 
                         (struct sockaddr*)&other, &addr_len);
                        
data[st] = '\0';
std::string pivot = std::string(data);
bool ip_cond = true;
std::string ip_pivot = "";
std::string name_pivot = "";
for(int i = 0; i<pivot.size(); i++){
    if(pivot[i] == ' '){
        ip_cond = false;
    }else{
        if(ip_cond){
         ip_pivot+= pivot[i];
        }else{
            name_pivot+= pivot[i];
        }
    }
}
ips[i] = strdup(ip_pivot.c_str());
art[i] = strdup(name_pivot.c_str());
std::cout<<"Recibí la siguiente información\n";
std::cout<<ips[i] <<"\n";
std::cout<<art[i] <<"\n";
}
    struct sockaddr_in server_addr_tcp;
    id = socket(AF_INET,SOCK_STREAM,0);
    //Simulamos un brodcast donde hacemos connect con todas las islas
    char* solicitud = "GET/FIGURES";
    for(int i = 0; i<6; i++){
    id = socket(AF_INET,SOCK_STREAM,0);
    memset(&server_addr_tcp, 0, sizeof(server_addr_tcp));
    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_addr.s_addr = INADDR_ANY;
    server_addr_tcp.sin_port = htons(ports[i]);
    server_addr_tcp.sin_addr.s_addr = inet_addr("127.0.0.1");
     // Nos conectamos al servidor remoto
     usleep(500000);
    if (connect(id, (struct sockaddr*)&server_addr_tcp, sizeof(server_addr_tcp)) < 0) {
        perror("Error al conectar con el servidor remoto");
        close(id);
          // Liberamos la memoria
        return;
    }
    sockets[i] = id;
    
    // Enviamos la solicitud de imagen al servidor remoto
    // yo en realidad solo tengo que conectarme de hecho de momento
    /*
    write(id, solicitud, strlen(solicitud));
    char* buffer = new char[200];
    int bytes_read = read(id,buffer,200);
    buffer[bytes_read] = '\0';
    art[i] = strdup(buffer);
    delete[] buffer;
    */
    }

}

void user_menu(char* art[], int* sockets){
    bool menu = true;
    int option = 0;
    char image_option[100];
    image_option[99] = '\0';
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
                if(strcmp(art[i] , image_option)==0){
                    flag = 1;
                    write(sockets[i],image_option,strlen(image_option));
                    char* buffer = new char[512];
                    read(sockets[i],buffer,512);
                    std::cout<<"Imagen ASCII Resultante:\n";
                    std::cout<<buffer <<"\n";
                    delete[] buffer;
                    break;
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
    char* figures_name[6];
    int servers_sockets[6];
    char* ip[6];
    server_connection_prove(figures_name,servers_sockets,ip);
    user_menu(figures_name,servers_sockets);
    for(int i = 0; i<6; i++){
        close(servers_sockets[i]);
    }
    return 0;
}