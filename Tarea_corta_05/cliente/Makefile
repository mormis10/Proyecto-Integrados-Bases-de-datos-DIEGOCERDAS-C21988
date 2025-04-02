# Linux Makefile
# Socket classes
#

CC  := gcc
CXX := g++
CFLAGS := -g -c
CXXFLAGS := -g -c
DEPS := Socket.h
TARGETS := cliente.cc 

ALL := $(TARGETS)

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $<

%.out: %.o Socket.o VSocket.o
	$(info $$^ = $^)
	$(CXX) -g $^ Socket.o VSocket.o -o $^

client.out: cliente.o Socket.o VSocket.o
	$(CXX) -g cliente.o Socket.o VSocket.o -o $@

clean:
	rm -f *.o *.out
