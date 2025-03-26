# Linux Makefile
# Socket class with SSL

CC  := gcc
CXX := g++
CFLAGS := -g -c -Wall
CXXFLAGS := -g -c -Wall
LDFLAGS := -lssl -lcrypto

DEPS := Socket.h VSocket.h SSLSocket.h
TARGET := tcp-cli.out

all: $(TARGET)

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $< -o $@

$(TARGET): tcp-cli.o VSocket.o Socket.o SSLSocket.o
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.o *.out
