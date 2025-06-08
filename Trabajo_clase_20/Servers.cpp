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
std::string figure_name;
std::string figure;
char* ip_direction;
int my_port;
int server_port;
int island_id;
pthread_mutex_t *shared_mutex;
}thread_data;

void *start_servers_connection(void* data){
    thread_data* tdata = (thread_data*)data;
    int sockid, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    sockid = socket(AF_INET,SOCK_STREAM,0);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(tdata->my_port);

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
    // Esta vaina es thread safe tons todo bien 
    pthread_mutex_lock(tdata->shared_mutex);
    std::cout<<"El hilo número: " <<tdata->island_id <<" ya está escuchando\n";
    pthread_mutex_unlock(tdata->shared_mutex);
    int client_id = accept(sockid, (struct sockaddr*)&client_addr, &client_len);
        if (client_id < 0) {
            perror("Error al aceptar conexión");
        }
    char buffer[200];
    read(client_id,buffer,200);
    pthread_mutex_lock(tdata->shared_mutex);
    std::cout<<"El server de la isal número: " <<tdata->island_id << "\n";
    std::cout<<"Recibió la siguiente solicitud: " <<buffer <<" \n";
    pthread_mutex_unlock(tdata->shared_mutex);
    pthread_mutex_lock(tdata->shared_mutex);
    std::cout<<"El server de la isal número: " <<tdata->island_id << "\n";
    std::cout<<"Recibió la siguiente solicitud: " <<buffer <<" \n";
    pthread_mutex_unlock(tdata->shared_mutex);
    //El read se queda esperando básicamete esa es la lógica
    const char* image = tdata->figure_name.c_str();
    pthread_mutex_lock(tdata->shared_mutex);
    std::cout<<image <<"\n";
    pthread_mutex_unlock(tdata->shared_mutex);
    write(client_id,image,strlen(image));
    char buffer2[200];
    read(client_id,buffer2,200);
    pthread_mutex_lock(tdata->shared_mutex);
    const char* image2 = tdata->figure.c_str();
    pthread_mutex_unlock(tdata->shared_mutex);
    write(client_id,image2,strlen(image2));
   // Ahora aquí tengo que enviar la data
}




int main(){
    pthread_t* threads = (pthread_t*) malloc(6*sizeof(pthread_t));
    thread_data* data = (thread_data*) malloc(6*sizeof(thread_data));
    std::string figure_name[6] = {"gatito","gokú","Batman","LDA","Kaká","Vale"};
    // Va a ser básico para estas pruebas del server
    std::string figure_content[6] = {"A","B","C","D","E","F"};
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