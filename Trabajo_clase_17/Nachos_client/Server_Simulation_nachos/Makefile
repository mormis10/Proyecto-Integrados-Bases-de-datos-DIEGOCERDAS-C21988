# Linux Makefile
# Socket class
#

CC  := gcc
CXX := g++
CFLAGS := -g -c
CXXFLAGS := -g -c
DEPS := Socket.h
TARGETS :=  ipv4-tcp-server-prove.out ipv-server-conteiner.out

all: $(TARGETS)
#$(info $$TARGETS = $(TARGETS))

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $<

%.out: %.o Socket.o VSocket.o
	$(info $$^ = $^)
	$(CXX) -g $^ VSocket.o Socket.o -o $^

ipv4-tcp-server-prove.out: ipv4-tcp-server-prove.o VSocket.o Socket.o
	$(CXX) -g ipv4-tcp-server-prove.o VSocket.o Socket.o -o $@

ipv-server-conteiner.out: ipv-server-conteiner.o VSocket.o Socket.o
	$(CXX) -g ipv-server-conteiner.o VSocket.o Socket.o -o $@

clean:
	rm -f *.o *.out
