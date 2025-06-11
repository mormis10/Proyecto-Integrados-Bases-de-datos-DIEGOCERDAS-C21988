#ifndef Socket_h
#define Socket_h

#include "VSocket.h"

class Socket : public VSocket {
 public:
  Socket(char, bool = false);
  Socket(int fd);
  ~Socket();

  size_t Read(void *, size_t);
  size_t Write(const void *, size_t);
  size_t Write(const char *);
  VSocket* AcceptConnection() override;
};

#endif  // Socket_h