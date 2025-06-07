# Server con protocolo HTTP

# Este servidor permite solicitudes de imagen ascci mediante un browser, ingresando la siguinte dirección

bash
```
localhost:1234/gatito
```


# en pantalla debería de aparecer el contenido solicitado por el usuario

# ejemplo de compilación:
bash
```
g++ -o prueba.out ThreadServer.cc VSocket.cc Socket.cc File_system.cc SuperBlock.cc 
```
# ejemplo de ejecución:

bash
```
./prueba.out
```



