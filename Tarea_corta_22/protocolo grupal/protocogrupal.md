# Protocolo de Comunicación entre Tenedores y Servidores de Figuras ASCII (Grupo 03)

## Introducción
Este documento describe el protocolo de comunicación utilizado entre los servidores de dibujos ASCII y los servidores tenedores. La comunicación entre **tenedores y servidores** se realiza mediante un **protocolo personalizado**, mientras que la interacción entre **clientes y tenedores** se efectúa mediante **HTTP**. La interacción cliente-tenedor queda fuera del alcance de este documento.

---

## 1. Descubrimiento de Servidores
### 1.1. Solicitud de descubrimiento (desde un tenedor nuevo)
Cuando un tenedor se conecta a la red, realiza un **broadcast** a los servidores con el siguiente mensaje:
```
Request: GET /servers
```

Los servidores activos que reciben esta solicitud responden directamente al tenedor que la envió, utilizando el siguiente formato:
```
Response: ServerName | ip | {lista_dibujos}
```

#### Ejemplo:
```
Response: ServidorA | 172.16.123.51 | ballena, perro, gato
```

> El tenedor guarda la información localmente, para luego hacer solicitudes de figuras al servidor que corresponda.
> Dado que se trata de un broadcast, otros tenedores que también lo reciban deben **ignorar** esta solicitud.

---

### 1.2. Anuncio de descubrimiento (desde un servidor nuevo)
Los servidores también deben anunciar cuando se conectan a la red mediante mensajes **broadcast** enviados a los tenedores en la red, utilizando el mismo formato de respuesta:
```
Response: ServerName | ip | {lista_dibujos}
```

#### Ejemplo:
```
Response: ServidorA | 172.16.123.51 | ballena, perro, gato
```

> Los demás servidores deben **ignorar** estos mensajes si los reciben, ya que están dirigidos únicamente a los tenedores.

---

## 2. Solicitud de Figuras ASCII
Cuando un tenedor desea obtener un dibujo específico, realiza una solicitud directa al servidor correspondiente:
```
Request: GET /figure/{nombre_figura}
```

### 2.1. Respuesta del servidor
- **Si la figura existe**, el servidor responde con el contenido del ASCII-art, sin encabezados:
```
(\__/)
(='.'=)
(")_(")
```

- **Si la figura no existe**, el tenedor, al tener una tabla local actualizada de las figuras disponibles por servidor, **no realiza la solicitud** y devuelve directamente:
```
404 Not Found
```

---

## 3. Notificación de Terminación de Servidor
Cuando un servidor va a finalizar su operación, envía un mensaje **broadcast** para notificar a los tenedores:
```
Request: Shutdown ServerName
```

#### Ejemplo:
```
Request: Shutdown ServidorA
```

> Los servidores que reciban esta notificación deben **ignorarla**.
> Cada tenedor que recibe este mensaje debe **eliminar** de su registro local toda la información asociada al servidor que se apaga.

---

## 4. Arquitectura del Servicio
### 4.1. Servidores Tenedores
- **Hilos:**
  - 1 hilo principal para gestión general
  - 5–10 hilos para atención de clientes HTTP
  - 1 hilo para descubrimiento de servidores

- **Puertos utilizados:**
  - `8080`: HTTP (clientes)
  - `5353`: Multicast (descubrimiento de servidores)
  - `9090`: Comunicación entre tenedores

---

### 4.2. Servidores de Figuras
- **Hilos:**
  - 1 hilo principal
  - 3–6 hilos para atender solicitudes
  - 1 hilo para latidos (heartbeat) con tenedores

- **Puertos utilizados:**
  - `8081`: HTTP (comunicación con tenedores)
  - `5353`: Multicast (anuncio a tenedores)
  - `7777`: Comunicación entre servidores

---

## 5. Distribución de Redes por Isla
- **Red pública general:** `10.1.35.0/24`
- **Islas:**
  - Isla 1: `172.16.123.16/28`
  - Isla 2: `172.16.123.32/28`
  - Isla 3: `172.16.123.48/28`
  - Isla 4: `172.16.123.64/28`
  - Isla 5: `172.16.123.80/28`
  - Isla 6: `172.16.123.96/28`
  - Isla Profesores: `172.16.123.0/28`
