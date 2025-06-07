# Simulation number 2

## Descripción

Esta simulación representa el protocolo de comunicación acordado por los compañeros para emular un modelo cliente-servidor usando múltiples hilos (threads) en C++.

El objetivo es simular el proceso de solicitud de una figura (imagen) por parte de un cliente. La solicitud se transmite a través de un "servidor tenedor", el cual mantiene una lista de servidores distribuidos (Islas) y las imágenes que poseen. El servidor tenedor es responsable de enrutar la solicitud hacia el servidor adecuado que contenga la imagen deseada. Una vez que la imagen es localizada, esta se transmite de vuelta al cliente.

Este escenario simula de manera didáctica el comportamiento de servicios distribuidos, sincronización con semáforos (`semaphores`), y la coordinación entre múltiples hilos.

## Componentes

- **Cliente**: Realiza la solicitud de una imagen específica.
- **Servidor Tenedor**: Recibe la solicitud del cliente y consulta la base de datos de servidores disponibles.
- **Servidor de imagen**: Responde con el contenido de la imagen, si está disponible.
- **Lista de servidores (Islas)**: Contienen distintos contenidos identificados por nombre y dirección IP.

## Compilación

Para compilar el archivo `Simulation_2.cc`, se debe utilizar el siguiente comando que incluye el enlace con la librería de hilos (`pthread`):

```bash
g++ -o simulation2.out Simulation_2.cc -pthread
