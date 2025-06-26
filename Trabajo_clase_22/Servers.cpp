#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>

// First prove for 

typedef struct{
char* figure_name;
char* figure;
char* ip_direction;
int my_port;
int server_port;
int island_id;
pthread_mutex_t *shared_mutex;
}thread_data;

void *start_servers_connection(void* data){
    //vamo a recibir
    thread_data* tdata = (thread_data*)data;
    int sock_udp = socket(AF_INET,SOCK_DGRAM,0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(tdata->my_port);
    
    if (bind(sock_udp, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        close(sock_udp);
        return nullptr;
    }
    struct sockaddr_in other;
    socklen_t addr_len = sizeof(other);
    char buffer_udp[201];
    ssize_t st = recvfrom(sock_udp, buffer_udp, 200, 0, 
                         (struct sockaddr*)&other, &addr_len);
    if (st < 0) {
        perror("Error en recvfrom");
        close(sock_udp);
        return nullptr;
    };
    buffer_udp[st] = '\0';
    pthread_mutex_lock(tdata->shared_mutex);
    std::cout<<"Mensaje recibido: " << buffer_udp <<"\n";
    pthread_mutex_unlock(tdata->shared_mutex);
   int sock_udp_send = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_udp_send < 0) {
        perror("Error al crear socket de envío");
        close(sock_udp);
        return nullptr;
    }

    struct sockaddr_in udp_mess;
    memset(&udp_mess, 0, sizeof(udp_mess)); 
    udp_mess.sin_port = htons(8080);         
    udp_mess.sin_family = AF_INET;
    
    if (inet_pton(AF_INET, "127.0.0.1", &udp_mess.sin_addr) <= 0) {
        perror("Error en inet_pton");
        close(sock_udp);
        close(sock_udp_send);
        return nullptr;
    }
    //usleep(50000000);
    const char *ip = "127.0.0.1";  // 9 bytes + null terminator
    const char* figureN = tdata->figure_name;
    std::string result = std::string(ip) + std::string(" ") + std::string(figureN);
    const char* message = result.c_str();
    ssize_t sent_bytes = sendto(sock_udp_send, message, strlen(message), 0, 
                               (const struct sockaddr *)&udp_mess, 
                               sizeof(udp_mess));
    
    if (sent_bytes < 0) {
        perror("Error en sendto");
    } else {
        pthread_mutex_lock(tdata->shared_mutex);
        std::cout << "Enviados " << sent_bytes << " bytes a 127.0.0.1:8080\n";
        pthread_mutex_unlock(tdata->shared_mutex);
    }    
    //Aquí inicia la parte tcp, hay que hacer la parte TCP
    int sockid, client_sockfd;
    struct sockaddr_in server_addr_tcp, client_addr;
    socklen_t client_len = sizeof(client_addr);
    sockid = socket(AF_INET,SOCK_STREAM,0);
    memset(&server_addr_tcp, 0, sizeof(server_addr_tcp));
    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_addr.s_addr = INADDR_ANY;
    server_addr_tcp.sin_port = htons(tdata->my_port);

    // Enlazar el socket con la dirección
    if (bind(sockid, (struct sockaddr*)&server_addr_tcp, sizeof(server_addr_tcp)) < 0) {
        perror("Error al enlazar socket");
        exit(1);
    }

    // Aquí empezamos a verificar que el servidor esté escuchando
    if (listen(sockid, 5) < 0) {
        perror("Error escuchando por solicitudes");
        exit(1);
    }
    // Esta vaina es thread safe tons todo bien 
    pthread_mutex_lock(tdata->shared_mutex);
    std::cout<<"El hilo número: " <<tdata->island_id <<" ya está escuchando\n";
    pthread_mutex_unlock(tdata->shared_mutex);
    int client_id = accept(sockid, (struct sockaddr*)&client_addr, &client_len);
        if (client_id < 0) {
            perror("Error al aceptar conexión");
        }

     pthread_mutex_lock(tdata->shared_mutex);
    std::cout<<"El hilo número: " <<tdata->island_id <<" acepta la conexión\n";
    pthread_mutex_unlock(tdata->shared_mutex);


      /*char buffer[200];
      int bytes_read = read(client_id,buffer,200);
      //int bytes_read = recvfrom(client_id, buffer, size, 0, (struct sockaddr *)addr, &addr_len);
    buffer[bytes_read] = '\0';
    pthread_mutex_lock(tdata->shared_mutex);
    std::cout<<"El server de la isal número: " <<tdata->island_id << "\n";
    std::cout<<"Recibió la siguiente solicitud: " <<buffer <<" \n";
    pthread_mutex_unlock(tdata->shared_mutex);
    pthread_mutex_lock(tdata->shared_mutex);
    std::cout<<"El server de la isal número: " <<tdata->island_id << "\n";
    std::cout<<"Recibió la siguiente solicitud: " <<buffer <<" \n";
    pthread_mutex_unlock(tdata->shared_mutex);
    //El read se queda esperando básicamete esa es la lógica
    const char* image = tdata->figure_name;
    pthread_mutex_lock(tdata->shared_mutex);
    std::cout<<image <<"\n";
    pthread_mutex_unlock(tdata->shared_mutex);
    write(client_id,image,strlen(image));
    */
    std::cout<<"llega aquí: " << tdata->island_id <<"\n";
    char buffer2[200] = {0};
     int bytes_read2 = read(client_id,buffer2,200);
     buffer2[bytes_read2] = '\0';
     printf("Recibo esta solicitud %s \n",buffer2);
    //pthread_mutex_lock(tdata->shared_mutex);
    const char* image2 = "   ******       ******\n"
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
    write(client_id,image2,strlen(image2));
    //pthread_mutex_unlock(tdata->shared_mutex);
     close(sock_udp);
    close(sock_udp_send);
}




int main(){
    pthread_t* threads = (pthread_t*) malloc(6*sizeof(pthread_t));
    thread_data* data = (thread_data*) malloc(6*sizeof(thread_data));
    char* figure_name[6] = {"gatito","gokú","Batman","LDA","Kaká","Vale"};
    // Va a ser básico para estas pruebas del server
    char* figure_content[6] = {"A","B","C","D","E","F"};
    int ports[6] = {8001,8002,8003,8004,8005,8006};
    char* ip = "127.0.0.1";
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    for(int i = 0; i<6; i++){
        data[i].figure = figure_content[i];
        data[i].figure_name = figure_name[i];
        data[i].my_port = ports[i];
        data[i].server_port = 8080;
        data[i].island_id = i+1;
        data[i].ip_direction = ip;
        data[i].shared_mutex = &mutex;
        pthread_create(&threads[i], NULL, start_servers_connection, &data[i]);
    }

    //Ahora creamos un socket para hacerle brodcast a todas las Islas
    
    // Hacemos el join de los hilos luego de que acaben su proceso
    for(int i = 0; i<6; i++){
        pthread_join(threads[i],NULL);
    }
    free(threads);

    return 0;
}