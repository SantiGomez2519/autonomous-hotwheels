# Autonomous Vehicle Telemetry System

This project implements a client-server system that simulates an autonomous vehicle that sends telemetry data (speed, battery, and temperature) and receives control commands. The system uses TCP sockets with Berkeley API and a custom application protocol.

## 👥 Authors

- **Santiago Gómez Ospina**
- **Isabella Camacho**
- **Sofía Flores**

## 📋 Table of Contents

- [🌟 Key Features](#-key-features)
- [🏗️ Project Structure](#️-project-structure)
- [🚀 Installation and Setup](#-installation-and-setup)
- [📖 Execution Instructions](#-execution-instructions)
- [🎮 System Usage](#-system-usage)
- [📊 Telemetry Data](#-telemetry-data)
- [📝 Communication Protocol](#-communication-protocol)
- [🔧 Makefile Commands](#-makefile-commands)
- [🐛 Troubleshooting](#-troubleshooting)
- [📋 Logging and Monitoring](#-logging-and-monitoring)
- [🧪 Testing](#-testing)
- [🔒 Security](#-security)
- [🏗️ Architecture](#️-architecture)
- [📞 Support](#-support)

## 🌟 Key Features

- **Modular C Server** with multi-client support using pthreads
- **Java Client** with Swing GUI (modular architecture)
- **Python Client** with Tkinter GUI (modular architecture)
- **Custom Protocol** inspired by RFC standards
- **Complete Logging** to console and file
- **Authentication System** for administrator users
- **Automatic Telemetry** every 10 seconds
- **Dynamic Battery System** with realistic consumption
- **Temperature Management** based on vehicle usage

## 🏗️ Project Structure

```
autonomous-hotwheels/
├── server/                    # C Server (Modular Architecture)
│   ├── server.c              # Main server file
│   ├── socket_manager.c/h    # Socket operations
│   ├── vehicle.c/h           # Vehicle state management
│   ├── client_protocol.c/h   # Client management + Protocol + Logging
│   └── Makefile              # Build configuration
├── client_java/              # Java Client (Modular)
│   ├── Main.java             # Main GUI interface
│   ├── NetworkManager.java   # Network communication
│   ├── VehicleData.java      # Vehicle data model
│   └── Makefile              # Build configuration
├── client_python/            # Python Client (Modular)
│   ├── main.py               # Main GUI interface
│   ├── network_manager.py    # Network communication
│   ├── vehicle_data.py       # Vehicle data model
│   └── Makefile              # Build configuration
├── docs/                     # Documentation
│   └── protocolo.md          # Protocol specification
└── README.md                 # This file
```

## 🚀 Installation and Setup

### System Requirements

- **Server**: GCC with pthread support
- **Python Client**: Python 3.6+ with tkinter
- **Java Client**: Java 8+ with Swing
- **Operating System**: Linux (for native socket and pthread support)
  
  Note: WSL can also be used to run the server on Windows.

### Verify Dependencies

```bash
# Check GCC and pthread
gcc --version
# Check Python and tkinter
python3 -c "import tkinter; print('tkinter available')"
# Check Java
java -version
```

### Update recommended packets (Ubuntu / Debian)

```bash
sudo apt update
sudo apt install build-essential
```

## 📖 Execution Instructions

### 1. Compile the Server

```bash
cd server
make
```

Or manually:

```bash
cd server
gcc -Wall -Wextra -std=c99 -o server server.c socket_manager.c vehicle.c client_protocol.c -lpthread
```

### 2. Run the Server

```bash
cd server
./server <port> <log_file>
```

**Example:**

```bash
./server 8080 server.log
```

The server:

- Listens on the specified port (e.g., 8080)
- Logs to the specified file (e.g., server.log)
- Supports up to 50 simultaneous clients
- Sends automatic telemetry every 10 seconds
- Default credentials: username `admin`, password `admin123`

### 3. Run Clients

#### Python Client

```bash
cd client_python
make run
```

Or directly:

```bash
cd client_python
python3 main.py
```

#### Java Client

```bash
cd client_java
make run
```

Or manually:

```bash
cd client_java
javac *.java
java Main
```

## 🎮 System Usage

### User Types

#### 👨‍💼 Administrator

- **Authentication required**: username `admin`, password `admin123`
- **Permissions**:
  - Send control commands (accelerate, brake, turn)
  - Query list of connected users
  - Recharge vehicle battery
  - Receive telemetry data

#### 👁️ Observer

- **No authentication required**
- **Permissions**:
  - Only receive and view telemetry data
  - Cannot send control commands

### Available Commands

| Command                       | Description                | User Type     |
| ----------------------------- | -------------------------- | ------------- |
| `AUTH <username> <password>`  | Authentication             | Administrator |
| `GET_DATA`                    | Request current data       | All           |
| `SEND_CMD <command>`          | Send control command       | Administrator |
| `RECHARGE`                    | Recharge vehicle battery   | Administrator |
| `LIST_USERS`                  | List connected users       | Administrator |
| `DISCONNECT`                  | Disconnect from server     | All           |

### Vehicle Control Commands

| Command      | Description     | Effect             |
| ------------ | --------------- | ------------------ |
| `SPEED_UP`   | Accelerate      | +10 km/h (max 100) |
| `SLOW_DOWN`  | Brake           | -10 km/h (min 0)   |
| `TURN_LEFT`  | Turn left       | Direction: LEFT    |
| `TURN_RIGHT` | Turn right      | Direction: RIGHT   |

## 📊 Telemetry Data

The server automatically sends every 10 seconds:

- **Speed**: 0-100 km/h
- **Battery**: 0-100% (dynamic consumption based on usage)
- **Temperature**: 20-50°C (varies with speed and usage)
- **Direction**: LEFT, RIGHT, STRAIGHT

### 🔋 Battery System

The battery system is now dynamic and realistic:

- **Base consumption**: 1% per minute when stationary
- **Speed consumption**: Additional 0.5% per minute per 10 km/h
- **Examples**:
  - Stationary (0 km/h): 1% per minute
  - 20 km/h: 2% per minute
  - 50 km/h: 3.5% per minute
  - 100 km/h: 6% per minute

### 🌡️ Temperature System

Temperature varies based on vehicle usage:

- **Heating**: Increases with speed
- **Cooling**: Decreases when stationary
- **Range**: 20°C (minimum) to 50°C (maximum)

## 📝 Communication Protocol

The system uses a custom text protocol over TCP:

### Message Format

```
<COMMAND>: <parameters>
USER: <username>
IP: <client_ip>
PORT: <client_port>
TIMESTAMP: <timestamp>

<message_body>
```

### Request Example

```
AUTH: admin admin123
USER: admin
IP: 192.168.1.100
PORT: 12345
TIMESTAMP: 2024-01-15 10:30:45
```

### Response Example

```
DATA: 45 85 23 LEFT
SERVER: telemetry_server
TIMESTAMP: 2024-01-15 10:31:16
```

For more details, see [docs/protocolo.md](docs/protocolo.md).

## 🔧 Makefile Commands

### Server Makefile

```bash
cd server

make          # Compile the server
make clean    # Remove compiled files
make run      # Run server (port 8080)
make help     # Show help
make install  # Install to /usr/local/bin
make uninstall# Uninstall
```

### Client Makefiles

```bash
# Java Client
cd client_java
make          # Compile Java client
make run      # Run Java client
make clean    # Clean compiled files

# Python Client
cd client_python
make check    # Check Python dependencies
make run      # Run Python client
```

## 🐛 Troubleshooting

### Server Compilation Error

```bash
# Check that pthread is available
gcc -v
# On Ubuntu/Debian if missing:
sudo apt-get install build-essential
```

### Python tkinter Error

```bash
# Ubuntu/Debian:
sudo apt-get install python3-tk
# CentOS/RHEL:
sudo yum install tkinter
```

### Connection Error

1. Verify that the server is running
2. Check that the port is not in use
3. Verify firewall and network permissions

### Port in Use

```bash
# Check what process uses the port
netstat -tulpn | grep :8080
# Or use lsof
lsof -i :8080
```

## 📋 Logging and Monitoring

### Server Log File

The server logs:

- Client connections and disconnections
- Commands received and responses sent
- System errors and events
- Timestamps and IP addresses

### Log Format

```
[2024-01-15 10:30:45] [192.168.1.100:12345] [CONNECT] Client connected
[2024-01-15 10:30:50] [192.168.1.100:12345] [COMMAND] AUTH: admin admin123
[2024-01-15 10:30:50] [192.168.1.100:12345] [RESPONSE] AUTH_SUCCESS
```

## 🧪 Testing

### Basic Test

1. Start the server: `./server 8080 server.log`
2. Connect Python client: `python3 main.py`
3. Authenticate as admin: username `admin`, password `admin123`
4. Send command: "Speed Up" button
5. Verify response in logs

### Multi-client Test

1. Start the server
2. Connect multiple clients (Python and Java)
3. Authenticate some as admin
4. Verify that all receive automatic telemetry
5. Test commands from different admin clients

### Battery System Test

1. Start the server and connect as admin
2. Accelerate the vehicle to high speed
3. Monitor battery consumption over time
4. Use the recharge command to restore battery
5. Verify temperature changes with speed

## 🔒 Security

- **Authentication**: Basic username/password system
- **Validation**: Commands and parameters are validated
- **Limits**: Maximum 50 concurrent clients
- **Timeouts**: Inactive clients are disconnected after 5 minutes

## 🏗️ Architecture

### Server Architecture

The server is built with a modular architecture:

- **`server.c`**: Main server file with connection handling
- **`socket_manager.c/h`**: Socket operations and network management
- **`vehicle.c/h`**: Vehicle state and telemetry management
- **`client_protocol.c/h`**: Client management, protocol handling, and logging

### Client Architecture

Both clients follow a similar modular pattern:

- **GUI Module**: User interface and event handling
- **Network Module**: Communication with the server
- **Data Module**: Vehicle data management

## 📞 Support

1. Review the documentation in `docs/protocolo.md`
2. Check server logs
3. Verify network and port configuration

---

**Developed for the Telematics course - EAFIT**
