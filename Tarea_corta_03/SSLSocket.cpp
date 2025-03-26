#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdexcept>
#include "SSLSocket.h"
#include "Socket.h"

SSLSocket::SSLSocket(char* hostip, int port, bool IPv6) {
   this->BuildSocket('s', IPv6);
   this->EstablishConnection(hostip, port);
   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;
   this->Init();
}

SSLSocket::SSLSocket(const char* hostip, const char* service, bool IPV6) {
   this->EstablishConnection(hostip, service, IPV6);
   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;
   this->Init();
}

SSLSocket::SSLSocket(int id) {
   this->BuildSocket(id);
   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;
   this->Init();
}

SSLSocket::~SSLSocket() {
   if (this->SSLStruct) {
      SSL_shutdown(reinterpret_cast<SSL*>(this->SSLStruct));
      SSL_free(reinterpret_cast<SSL*>(this->SSLStruct));
   }
   if (this->SSLContext) {
      SSL_CTX_free(reinterpret_cast<SSL_CTX*>(this->SSLContext));
   }
   this->Close();
}

void SSLSocket::Init(bool serverContext) {
   this->InitContext(serverContext);
   this->SSLStruct = SSL_new(reinterpret_cast<SSL_CTX*>(this->SSLContext));
   if (!this->SSLStruct) {
      throw std::runtime_error("Error creando SSL struct");
   }
   SSL_set_fd(reinterpret_cast<SSL*>(this->SSLStruct), this->idSocket);
   if (serverContext) {
      if (SSL_accept(reinterpret_cast<SSL*>(this->SSLStruct)) <= 0) {
         throw std::runtime_error("Error al aceptar conexión SSL");
      }
   } else {
      if (SSL_connect(reinterpret_cast<SSL*>(this->SSLStruct)) <= 0) {
         throw std::runtime_error("Error al conectar SSL");
      }
   }
}

void SSLSocket::InitContext(bool serverContext) {
   const SSL_METHOD* method;
   if (serverContext) {
      method = TLS_server_method();
   } else {
      method = TLS_client_method();
   }
   SSL_CTX* context = SSL_CTX_new(method);
   if (!context) {
      throw std::runtime_error("No se pudo crear el contexto SSL");
   }
   this->SSLContext = reinterpret_cast<void*>(context);
}

void SSLSocket::LoadCertificates(const char* certFileName, const char* keyFileName) {
   SSL_CTX* ctx = reinterpret_cast<SSL_CTX*>(this->SSLContext);
   if (SSL_CTX_use_certificate_file(ctx, certFileName, SSL_FILETYPE_PEM) <= 0) {
      throw std::runtime_error("Error cargando el certificado");
   }
   if (SSL_CTX_use_PrivateKey_file(ctx, keyFileName, SSL_FILETYPE_PEM) <= 0) {
      throw std::runtime_error("Error cargando la llave privada");
   }
   if (!SSL_CTX_check_private_key(ctx)) {
      throw std::runtime_error("La llave privada no concuerda con el certificado");
   }
}

int SSLSocket::Connect(const char* hostName, int port) {
   return this->EstablishConnection(hostName, port);
}

int SSLSocket::Connect(const char* hostName, const char* service) {
   return this->EstablishConnection(hostName, service, false);
}

size_t SSLSocket::Write(const char* string) {
   int st = SSL_write(reinterpret_cast<SSL*>(this->SSLStruct), string, strlen(string));
   if (st <= 0) {
      throw std::runtime_error("Error escribiendo en el canal SSL");
   }
   return static_cast<size_t>(st);
}

size_t SSLSocket::Write(const void* buffer, size_t size) {
   int written_bytes = SSL_write(reinterpret_cast<SSL*>(this->SSLStruct), buffer, size);
   if (written_bytes <= 0) {
      throw std::runtime_error("Error escribiendo datos en SSL");
   }
   return static_cast<size_t>(written_bytes);
}

size_t SSLSocket::Read(void* buffer, size_t size) {
   int read_bytes = SSL_read(reinterpret_cast<SSL*>(this->SSLStruct), buffer, size);
   if (read_bytes <= 0) {
      throw std::runtime_error("Error leyendo datos de SSL");
   }
   return static_cast<size_t>(read_bytes);
}

void SSLSocket::ShowCerts() {
   X509* cert = SSL_get_peer_certificate(reinterpret_cast<SSL*>(this->SSLStruct));
   if (cert) {
      char* line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
      printf("Subject: %s\n", line);
      free(line);
      line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
      printf("Issuer: %s\n", line);
      free(line);
      X509_free(cert);
   } else {
      printf("No certificates.\n");
   }
}

const char* SSLSocket::GetCipher() {
   return SSL_get_cipher(reinterpret_cast<SSL*>(this->SSLStruct));
}
