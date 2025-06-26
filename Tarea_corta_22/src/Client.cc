#include <stdio.h>
#include <cstring>
#include <iostream>
#include "Socket.h"

#define PORT 8080    // Puerto del Tenedor
#define BUFSIZE 512

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " LIST/GET <figure_name>\n";
    return 1;
  }

  VSocket *s = new Socket('s', false);  // IPv4
  s->EstablishConnection("10.1.35.23", PORT);  // Cambiar IP si el tenedor está en otra máquina

  std::string request;
  if (strcmp(argv[1], "LIST") == 0) {
    request = "GET /LIST HTTP/1.1\r\nHost: localhost\r\n\r\n";
  } else if (strcmp(argv[1], "GET") == 0 && argc == 3) {
    request = "GET /" + std::string(argv[2]) + " HTTP/1.1\r\nHost: localhost\r\n\r\n";
  } else {
    std::cerr << "Invalid command. Use: LIST/GET <figure_name>\n";
    return 1;
  }

  s->Write(request.c_str(), request.size());

  char buffer[BUFSIZE];
  int bytesRead;
  while ((bytesRead = s->Read(buffer, BUFSIZE)) > 0) {
    fwrite(buffer, 1, bytesRead, stdout);
  }

  s->Close();
  return 0;
}
