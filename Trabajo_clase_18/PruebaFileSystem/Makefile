# Compilador y flags
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Archivos fuente
SRCS = main.cpp File_system.cc SuperBlock.cc
OBJS = $(SRCS:.cc=.o)
TARGET = main

# Regla por defecto
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o $(TARGET)