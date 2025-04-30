# Trabajo_clase # 8


## Compilación
# Hola nath, para esta entrega hay dos archivos a compilar

# Caso threads: 
```bash
g++ -o server SSLServer.cc VSocket.cc SSLSocket.cpp -lssl -lcrypto
g++ -o client SSLClient.cc VSocket.cc SSLSocket.cpp -lssl -lcrypto
```

# eso genera dos ejecutables que hay que emplear en dos terminales distintas, y en el caso del servidor debes de ingresar la contraseña "ci0123"

## Ejecución

# para ejecutar este proyecto se deben de ingresar los siguientes comandos en distintas terminales:

# Caso threads:
```bash
./server.out
./cliente.out
```
