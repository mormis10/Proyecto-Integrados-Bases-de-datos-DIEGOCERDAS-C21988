# Tarea corta# 8


## Compilación
# Hola nath, para esta entrega podés ejecutar el server con threads o con fork

# Caso threads: 
```bash
g++ -o prueba2.out VSocket.cc Socket.cc MirrorClient.cc
g++ -o prueba.out VSocket.cc Socket.cc ThreadMirrorServer.cc
```

# Caso fork: 
```bash
g++ -o prueba2.out VSocket.cc Socket.cc MirrorClient.cc
g++ -o prueba1.out VSocket.cc Socket.cc ForkMirrorServer.cc
```
# eso genera dos ejecutables que hay que emplear en dos terminales distintas

## Ejecución

# para ejecutar este proyecto se deben de ingresar los siguientes comandos en distintas terminales:

# Caso threads:
```bash
./prueba.out
./prueba2.out
```
# Caso fork
```bash
./prueba1.out
./prueba2.out
```a2.out
```