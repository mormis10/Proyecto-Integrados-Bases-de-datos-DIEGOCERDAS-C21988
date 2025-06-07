#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iostream>

typedef struct {
  sem_t server_haver;
  sem_t client;
  sem_t server;
  char *solicitud;
  char *respuesta;
} threads_data;

typedef struct {
  char *request;
  threads_data *shared_data;
} client_data;

typedef struct {
  const char *client_request;
  threads_data *shared_data;
} haver_data;

typedef struct {
  char *heart;
  threads_data *shared_data;
} server_data;

typedef struct 
{
  std::string server_name;
  std::string server_ip;
  std::string server_content;
}Servers_data;


void *client(void *data) {
  client_data *data_c = (client_data *)data;
  printf("El cliente se ha conectado al servidor tenedor y ha realizado una solicitud\n");
  data_c->shared_data->solicitud = data_c->request;
  usleep(4000000);
  sem_post(&data_c->shared_data->server_haver);
  sem_wait(&data_c->shared_data->client);
  printf("%s\n", data_c->shared_data->respuesta);
  printf("El cliente ha recibido la información con éxito\n");
}

void *server_haver(void *data) {
  haver_data *data_h = (haver_data *)data;
  sem_wait(&data_h->shared_data->server_haver);
  printf("El servidor tenedor ha recibido la solicitud del usuario\n");
  usleep(4000000);
  printf("%s\n", data_h->shared_data->solicitud);
  sem_post(&data_h->shared_data->server);
  sem_wait(&data_h->shared_data->server_haver);
  printf("El servidor de dibujo se contactó con el servidor tenedor\n");
  usleep(4000000);
  sem_post(&data_h->shared_data->client);
}

void set_server_data(Servers_data* servers){
  //Aquí vamos a definir 
  servers[0].server_name = "Isla1";
  servers[0].server_ip = "172.16.123.16/28";
  servers[0].server_content = "Corazón";

  servers[1].server_name = "Isla2";
  servers[1].server_ip = "172.16.123.32/28";
  servers[1].server_content = "gatito";

  servers[2].server_name = "Isla3";
  servers[2].server_ip = "172.16.123.48/28";
  servers[2].server_content = "perrito";

  servers[3].server_name = "Isla4";
  servers[3].server_ip = "172.16.123.64/28";
  servers[3].server_content = "zaguate";

  servers[4].server_name = "Isla5";
  servers[4].server_ip = "172.16.123.80/28";
  servers[4].server_content = "gokú";

  servers[5].server_name = "Isla6";
  servers[5].server_ip = "172.16.123.96/28";
  servers[5].server_content = "batman";
}

void show_server_content(Servers_data* servers, std::vector<std::string> &values) {
  for (int i = 0; i < 6; i++) {
    printf("Recibimos el mensaje, somos el servidor %s y tenemos la imagen: %s\n",
           servers[i].server_name.c_str(), servers[i].server_content.c_str());
    values.push_back(servers[i].server_content);
    usleep(300000);  
  }
}

void brodcast(){
  printf("Hola envío este mensaje a todos los servidores para que me digan cuales imagenes poseen\n");
  usleep(100000);
}

void *server_m(void *data) {
  server_data *data_s = (server_data *)data;
  sem_wait(&data_s->shared_data->server);
  printf("Solicitud encontrada con éxito\n");
  usleep(4000000);
  data_s->shared_data->respuesta = data_s->heart;
  sem_post(&data_s->shared_data->server_haver);
}

bool get_server_with_content(std::string content,Servers_data* server){
  
  for(int i = 0; i<6; i++){
    if(content == server[i].server_content){
      printf("SÍ lo poseo\n");
      printf("El server que lo posee es la: %s",
           server[i].server_name.c_str());
           printf("\n");
           return true;
    }
  }

  return false;
}
int main() {
  pthread_t clientt, haver_server, server_t;
  threads_data *data = (threads_data *)malloc(sizeof(threads_data));
  client_data *data_c = (client_data *)malloc(sizeof(client_data));
  haver_data *data_h = (haver_data *)malloc(sizeof(haver_data));
  server_data *data_s = (server_data *)malloc(sizeof(server_data));
  brodcast();
  Servers_data servers[6];
  set_server_data(servers);
  std::vector<std::string>values;
  show_server_content(servers,values);
  usleep(200000);
  printf("Holaa soy un cliente y me gustaría jalar una\n");
  std::string entry;
  std::cin>>entry;
  printf("Hola voy a revisar si tengo esa imagen\n");
  bool status = get_server_with_content(entry,servers);
  if(!status){
    printf("404 Not Found");
    return 0;
  }
  data_c->shared_data = data;
  data_c->request = "GET /Corazón.txt";
  data_h->shared_data = data;
  data_s->shared_data = data;
  data_s->heart = "   ******       ******\n"
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

  sem_init(&data->client, 0, 0);
  sem_init(&data->server, 0, 0);
  sem_init(&data->server_haver, 0, 0);
  pthread_create(&clientt, NULL, client, data_c);
  pthread_create(&haver_server, NULL, server_haver, data_h);
  pthread_create(&server_t, NULL, server_m, data_s);

  pthread_join(clientt, NULL);
  pthread_join(haver_server, NULL);
  pthread_join(server_t, NULL);

  // Destruye semáforos
  sem_destroy(&data->server_haver);
  sem_destroy(&data->client);
  sem_destroy(&data->server);
  free(data);
  free(data_c);
  free(data_h);
  free(data_s);
}