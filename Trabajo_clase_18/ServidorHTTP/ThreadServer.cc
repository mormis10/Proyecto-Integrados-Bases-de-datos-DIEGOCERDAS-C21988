#include <iostream>
#include <string.h>
#include <string>
#include <thread>
#include "File_system.h"

#include "Socket.h"

#define PORT 1234
#define BUFSIZE 512

std::string decode_url(const std::string &url) {
  std::string decoded;
  for (size_t i = 0; i < url.length(); ++i) {
    if (url[i] == '%') {
      char hex[3] = {url[i + 1], url[i + 2], '\0'};
      decoded += static_cast<char>(std::stoi(hex, nullptr, 16));
      i += 2;
    } else if (url[i] == '+') {
      decoded += ' ';
    } else {
      decoded += url[i];
    }
  }
  return decoded;
}

void task(VSocket *client) {
  char a[512];
  printf("LLega\n");

  client->Read(a, 512);
  std::cout << "Server received:\n" << a << std::endl;

  File_system fs;
  std::string request(a);
  size_t method_end = request.find(" ");
  size_t path_end = request.find(" ", method_end + 1);
  std::string method = request.substr(0, method_end);
  std::string path = request.substr(method_end + 1, path_end - method_end - 1);

  path = decode_url(path);

  if (method == "GET") {
    if (path == "/gatito") {
      char *imagen_ascii = fs.read_file(path.c_str());
      size_t content_length = strlen(imagen_ascii);

    char http_response[BUFSIZE * 3];
    snprintf(http_response, sizeof(http_response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/plain; charset=utf-8\r\n"
             "Content-Length: %lu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             strlen(imagen_ascii), imagen_ascii);

    client->Write(http_response);
    }
  } else {
    char error_response[] = "HTTP/1.1 405 Method Not Allowed\r\n"
                            "Content-Type: text/plain\r\n"
                            "\r\n"
                            "405 Method Not Allowed";
    client->Write(error_response);
  }

  client->Close();
}

void taskc(VSocket *client) {
  char a[BUFSIZE];
  printf("LLega\n");

  client->Read(a, BUFSIZE);
  std::cout << "Server received: " << a << std::endl;
  char response[BUFSIZE] = "MIIILAN!!";
  client->Write(response);
  client->Close();
}

int main(int argc, char **argv) {
  std::thread *worker;
  VSocket *s1, *client;

  s1 = new Socket('s');

  if (s1->Bind(PORT) == -1) {
    std::cerr << "Error al enlazar el puerto" << std::endl;
    return -1;
  }

  s1->MarkPassive(5);
  for (;;) {
    client = s1->AcceptConnection();
    worker = new std::thread(task, client);
    worker->join();
  }
  return 0;
}
