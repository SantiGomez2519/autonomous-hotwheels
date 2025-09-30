# Protocolo de Aplicación - Sistema de Telemetría Vehículo Autónomo

## 1. Visión General

### Propósito

Este protocolo define la comunicación entre un servidor de telemetría y múltiples clientes para simular un sistema de vehículo autónomo. El protocolo permite el envío de datos de telemetría (velocidad, batería, temperatura) y el control del vehículo mediante comandos.

### Modelo Cliente-Servidor

- **Servidor**: Mantiene el estado del vehículo, procesa comandos y envía datos de telemetría
- **Cliente Administrador**: Puede enviar comandos de control y consultar usuarios conectados
- **Cliente Observador**: Solo recibe datos de telemetría

### Capa de Aplicación

El protocolo funciona sobre TCP (SOCK_STREAM) para garantizar la confiabilidad de las comunicaciones, especialmente importante para comandos de control.

## 2. Especificación del Servicio

### Comandos Disponibles

#### Para Clientes Administradores:

- `AUTH <username> <password>` - Autenticación de administrador
- `GET_DATA` - Solicitar datos de telemetría actuales
- `SEND_CMD <command>` - Enviar comando de control
- `LIST_USERS` - Listar usuarios conectados
- `DISCONNECT` - Desconectar del servidor

#### Para Clientes Observadores:

- `GET_DATA` - Solicitar datos de telemetría actuales
- `DISCONNECT` - Desconectar del servidor

#### Comandos de Control del Vehículo:

- `SPEED_UP` - Aumentar velocidad
- `SLOW_DOWN` - Disminuir velocidad
- `TURN_LEFT` - Girar a la izquierda
- `TURN_RIGHT` - Girar a la derecha

### Respuestas del Servidor:

- `OK <message>` - Comando ejecutado exitosamente
- `ERROR <message>` - Error en el comando
- `DATA <speed> <battery> <temperature> <direction>` - Datos de telemetría
- `USERS <list>` - Lista de usuarios conectados
- `AUTH_SUCCESS` - Autenticación exitosa
- `AUTH_FAILED` - Autenticación fallida

## 3. Formato de Mensajes

### Estructura General

```
<COMMAND>: <parameters>
USER: <username>
IP: <client_ip>
PORT: <client_port>
TIMESTAMP: <timestamp>

<body>
```

### Ejemplos de Mensajes

#### Solicitud de Autenticación:

```
AUTH: admin password123
USER: admin
IP: 192.168.1.100
PORT: 12345
TIMESTAMP: 2024-01-15 10:30:45
```

#### Comando de Control:

```
SEND_CMD: SPEED_UP
USER: admin
IP: 192.168.1.100
PORT: 12345
TIMESTAMP: 2024-01-15 10:31:00
```

#### Solicitud de Datos:

```
GET_DATA:
USER: observer1
IP: 192.168.1.101
PORT: 12346
TIMESTAMP: 2024-01-15 10:31:15
```

#### Respuesta de Datos:

```
DATA: 45 85 23 LEFT
SERVER: telemetry_server
TIMESTAMP: 2024-01-15 10:31:16
```

#### Respuesta de Error:

```
ERROR: Invalid command
SERVER: telemetry_server
TIMESTAMP: 2024-01-15 10:31:20
```

## 4. Reglas de Procedimiento

### Estados del Cliente:

1. **CONNECTED** - Cliente conectado, no autenticado
2. **AUTHENTICATED** - Cliente autenticado como administrador
3. **OBSERVER** - Cliente en modo observador
4. **DISCONNECTED** - Cliente desconectado

### Flujo de Comunicación:

#### Para Administradores:

1. Cliente se conecta → Estado: CONNECTED
2. Cliente envía AUTH → Servidor valida credenciales
3. Si válido → Estado: AUTHENTICATED, respuesta: AUTH_SUCCESS
4. Si inválido → Estado: CONNECTED, respuesta: AUTH_FAILED
5. Cliente puede enviar comandos de control
6. Servidor responde con OK/ERROR
7. Cliente puede solicitar LIST_USERS

#### Para Observadores:

1. Cliente se conecta → Estado: CONNECTED
2. Cliente puede solicitar GET_DATA inmediatamente → Estado: OBSERVER
3. Servidor envía datos periódicamente cada 10 segundos

### Manejo de Errores:

- Comandos inválidos: Respuesta ERROR con descripción
- Cliente desconectado: Remover de lista de usuarios
- Mensaje malformado: Ignorar y continuar
- Timeout: Cerrar conexión

## 5. Ejemplos de Implementación

### Secuencia de Autenticación de Administrador:

```
Cliente → Servidor: AUTH admin password123
Servidor → Cliente: AUTH_SUCCESS
Cliente → Servidor: SEND_CMD SPEED_UP
Servidor → Cliente: OK Speed increased to 50 km/h
Cliente → Servidor: GET_DATA
Servidor → Cliente: DATA 50 85 23 LEFT
```

### Secuencia de Cliente Observador:

```
Cliente → Servidor: GET_DATA
Servidor → Cliente: DATA 45 85 23 LEFT
[Servidor envía datos cada 10 segundos automáticamente]
Servidor → Cliente: DATA 47 84 24 STRAIGHT
```

### Secuencia de Listado de Usuarios:

```
Cliente → Servidor: LIST_USERS
Servidor → Cliente: USERS admin(192.168.1.100:12345) observer1(192.168.1.101:12346)
```

## 6. Especificaciones Técnicas

### Codificación:

- Texto plano ASCII
- Terminación de línea: \r\n (CRLF)
- Codificación: UTF-8

### Timeouts:

- Conexión: 30 segundos
- Comando: 10 segundos
- Telemetría: 10 segundos (automática)

### Límites:

- Máximo 50 clientes concurrentes
- Máximo 1024 bytes por mensaje
- Credenciales persistentes por IP

### Logging:

- Todos los mensajes se registran con timestamp
- Formato: [TIMESTAMP] [IP:PORT] [TYPE] [MESSAGE]
- Tipos: CONNECT, DISCONNECT, COMMAND, RESPONSE, ERROR

## 7. Implementación de Seguridad

### Autenticación:

- Usuario por defecto: admin
- Contraseña por defecto: admin123
- Autenticación basada en IP persistente
- Sesiones no expiran

### Validación:

- Verificar formato de comandos
- Validar parámetros numéricos
- Sanitizar entradas de usuario
- Límites de velocidad y temperatura

---

**Versión del Protocolo**: 1.0  
**Fecha**: 2024-01-15  
**Autor**: Sistema de Telemetría Vehículo Autónomo
