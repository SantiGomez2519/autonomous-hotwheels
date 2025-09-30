# Sistema de Telemetría Vehículo Autónomo

Este proyecto implementa un sistema cliente-servidor que simula un vehículo autónomo que envía datos de telemetría (velocidad, batería y temperatura) y recibe comandos de control. El sistema utiliza sockets TCP con la API Berkeley y un protocolo de aplicación personalizado.

## 📋 Características

- **Servidor en C** con soporte multicliente usando hilos (pthread)
- **Cliente Python** con interfaz gráfica usando tkinter
- **Cliente Java** con interfaz gráfica usando Swing
- **Protocolo personalizado** inspirado en RFC
- **Logging completo** en consola y archivo
- **Autenticación** para usuarios administradores
- **Telemetría automática** cada 10 segundos

## 🏗️ Estructura del Proyecto

```
autonomous-hotwheels/
├── server/                 # Servidor en C
│   ├── server.c           # Código fuente del servidor
│   └── Makefile           # Archivo de compilación
├── client_python/         # Cliente Python
│   ├── client.py          # Lógica del cliente
│   ├── gui.py             # Interfaz gráfica
│   └── requirements.txt   # Dependencias (ninguna adicional)
├── client_java/           # Cliente Java
│   └── Client.java        # Cliente completo con GUI
├── docs/                  # Documentación
│   └── protocolo.md       # Especificación del protocolo
└── README.md              # Este archivo
```

## 🚀 Instalación y Configuración

### Requisitos del Sistema

- **Servidor**: GCC con soporte para pthread
- **Cliente Python**: Python 3.6+ con tkinter
- **Cliente Java**: Java 8+ con Swing

### Verificar Dependencias

```bash
# Verificar GCC y pthread
gcc --version
# Verificar Python y tkinter
python3 -c "import tkinter; print('tkinter disponible')"
# Verificar Java
java -version
```

## 📖 Instrucciones de Ejecución

### 1. Compilar el Servidor

```bash
cd server
make
```

O manualmente:

```bash
cd server
gcc -Wall -Wextra -std=c99 -o server server.c -lpthread
```

### 2. Ejecutar el Servidor

```bash
cd server
./server <puerto> <archivo_log>
```

**Ejemplo:**

```bash
./server 8080 server.log
```

El servidor:

- Escucha en el puerto especificado (ej: 8080)
- Registra logs en el archivo especificado (ej: server.log)
- Soporta hasta 50 clientes simultáneos
- Envía telemetría automática cada 10 segundos
- Credenciales por defecto: usuario `admin`, contraseña `admin123`

### 3. Ejecutar Clientes

#### Cliente Python

```bash
cd client_python
python3 client.py
```

O ejecutar directamente la GUI:

```bash
cd client_python
python3 -c "from client import main; main()"
```

#### Cliente Java

```bash
cd client_java
javac Client.java
java Client
```

## 🎮 Uso del Sistema

### Tipos de Usuario

#### 👨‍💼 Administrador

- **Autenticación requerida**: usuario `admin`, contraseña `admin123`
- **Permisos**:
  - Enviar comandos de control (acelerar, frenar, girar)
  - Consultar lista de usuarios conectados
  - Recibir datos de telemetría

#### 👁️ Observador

- **Sin autenticación requerida**
- **Permisos**:
  - Solo recibe y visualiza datos de telemetría
  - No puede enviar comandos de control

### Comandos Disponibles

| Comando                       | Descripción                | Usuario       |
| ----------------------------- | -------------------------- | ------------- |
| `AUTH <usuario> <contraseña>` | Autenticación              | Administrador |
| `GET_DATA`                    | Solicitar datos actuales   | Todos         |
| `SEND_CMD <comando>`          | Enviar comando de control  | Administrador |
| `LIST_USERS`                  | Listar usuarios conectados | Administrador |
| `DISCONNECT`                  | Desconectar del servidor   | Todos         |

### Comandos de Control del Vehículo

| Comando      | Descripción     | Efecto             |
| ------------ | --------------- | ------------------ |
| `SPEED_UP`   | Acelerar        | +10 km/h (máx 100) |
| `SLOW_DOWN`  | Frenar          | -10 km/h (mín 0)   |
| `TURN_LEFT`  | Girar izquierda | Dirección: LEFT    |
| `TURN_RIGHT` | Girar derecha   | Dirección: RIGHT   |

## 📊 Datos de Telemetría

El servidor envía automáticamente cada 10 segundos:

- **Velocidad**: 0-100 km/h
- **Batería**: 0-100%
- **Temperatura**: 20-30°C (simulada)
- **Dirección**: LEFT, RIGHT, STRAIGHT

## 📝 Protocolo de Comunicación

El sistema utiliza un protocolo de texto personalizado sobre TCP:

### Formato de Mensaje

```
<COMANDO>: <parámetros>
USER: <usuario>
IP: <ip_cliente>
PORT: <puerto_cliente>
TIMESTAMP: <timestamp>

<cuerpo_del_mensaje>
```

### Ejemplo de Solicitud

```
AUTH: admin admin123
USER: admin
IP: 192.168.1.100
PORT: 12345
TIMESTAMP: 2024-01-15 10:30:45
```

### Ejemplo de Respuesta

```
DATA: 45 85 23 LEFT
SERVER: telemetry_server
TIMESTAMP: 2024-01-15 10:31:16
```

Para más detalles, consulta [docs/protocolo.md](docs/protocolo.md).

## 🔧 Comandos del Makefile

```bash
cd server

make          # Compilar el servidor
make clean    # Eliminar archivos compilados
make run      # Ejecutar servidor (puerto 8080)
make help     # Mostrar ayuda
make install  # Instalar en /usr/local/bin
make uninstall# Desinstalar
```

## 🐛 Solución de Problemas

### Error de Compilación del Servidor

```bash
# Verificar que pthread esté disponible
gcc -v
# En Ubuntu/Debian si falta:
sudo apt-get install build-essential
```

### Error de tkinter en Python

```bash
# Ubuntu/Debian:
sudo apt-get install python3-tk
# CentOS/RHEL:
sudo yum install tkinter
```

### Error de Conexión

1. Verificar que el servidor esté ejecutándose
2. Comprobar que el puerto no esté en uso
3. Verificar firewall y permisos de red

### Puerto en Uso

```bash
# Verificar qué proceso usa el puerto
netstat -tulpn | grep :8080
# O usar lsof
lsof -i :8080
```

## 📋 Logs y Monitoreo

### Archivo de Logs del Servidor

El servidor registra:

- Conexiones y desconexiones de clientes
- Comandos recibidos y respuestas enviadas
- Errores y eventos del sistema
- Timestamps y direcciones IP

### Formato de Log

```
[2024-01-15 10:30:45] [192.168.1.100:12345] [CONNECT] Cliente conectado
[2024-01-15 10:30:50] [192.168.1.100:12345] [COMMAND] AUTH: admin admin123
[2024-01-15 10:30:50] [192.168.1.100:12345] [RESPONSE] AUTH_SUCCESS
```

## 🧪 Pruebas

### Prueba Básica

1. Iniciar el servidor: `./server 8080 server.log`
2. Conectar cliente Python: `python3 client.py`
3. Autenticar como admin: usuario `admin`, contraseña `admin123`
4. Enviar comando: botón "Acelerar"
5. Verificar respuesta en logs

### Prueba Multicliente

1. Iniciar el servidor
2. Conectar múltiples clientes (Python y Java)
3. Autenticar algunos como admin
4. Verificar que todos reciben telemetría automática
5. Probar comandos desde diferentes clientes admin

## 🔒 Seguridad

- **Autenticación**: Sistema básico de usuario/contraseña
- **Validación**: Comandos y parámetros son validados
- **Límites**: Máximo 50 clientes concurrentes
- **Timeouts**: Clientes inactivos son desconectados después de 5 minutos

## 🤝 Contribución

1. Fork el proyecto
2. Crear una rama para tu feature (`git checkout -b feature/nueva-funcionalidad`)
3. Commit tus cambios (`git commit -am 'Agregar nueva funcionalidad'`)
4. Push a la rama (`git push origin feature/nueva-funcionalidad`)
5. Crear un Pull Request

## 📄 Licencia

Este proyecto es para fines educativos como parte del curso de Telemática.

## 📞 Soporte

Para problemas o preguntas:

1. Revisar la documentación en `docs/protocolo.md`
2. Verificar los logs del servidor
3. Comprobar la configuración de red y puertos

---

**Desarrollado para el curso de Telemática - EAFIT**
