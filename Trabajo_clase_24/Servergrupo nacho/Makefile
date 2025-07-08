# Compilador y flags
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -pthread

# Archivos fuente comunes
COMMON_SRCS := \
    Figure_server.cc \
    File_system.cc \
    Fork_Server.cc \
    Socket.cc \
    SuperBlock.cc \
    VSocket.cc

# Archivos objeto comunes
COMMON_OBJS := $(COMMON_SRCS:.cc=.o)

# Ejecutables
TARGETS := main_figure main_fork client_udp main_manage_figures

# Reglas principales
all: $(TARGETS)

main_figure: main_figure.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

main_fork: main_fork.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

client_udp: client_udp.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

main_manage_figures : main_manage_figures.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regla para compilar archivos fuente a objeto
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f *.o $(TARGETS)

.PHONY: all clean