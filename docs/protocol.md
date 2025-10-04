# Application Protocol - Autonomous Vehicle Telemetry System

## 1. Overview

### Purpose

This protocol defines the communication between a telemetry server and multiple clients to simulate an autonomous vehicle system. The protocol allows sending telemetry data (speed, battery, temperature) and vehicle control through commands.

### Client-Server Model

- **Server**: Maintains vehicle state, processes commands and sends telemetry data
- **Administrator Client**: Can send control commands and query connected users
- **Observer Client**: Only receives telemetry data

### Application Layer

The protocol works over TCP (SOCK_STREAM) to ensure communication reliability, especially important for control commands.

## 2. Service Specification

### Available Commands

#### For Administrator Clients:

- `AUTH <username> <password>` - Administrator authentication
- `GET_DATA` - Request current telemetry data
- `SEND_CMD <command>` - Send control command
- `RECHARGE` - Recharge vehicle battery
- `LIST_USERS` - List connected users
- `DISCONNECT` - Disconnect from server

#### For Observer Clients:

- `GET_DATA` - Request current telemetry data
- `DISCONNECT` - Disconnect from server

#### Vehicle Control Commands:

- `SPEED_UP` - Increase speed
- `SLOW_DOWN` - Decrease speed
- `TURN_LEFT` - Turn left
- `TURN_RIGHT` - Turn right

### Server Responses:

- `OK <message>` - Command executed successfully
- `ERROR <message>` - Command error
- `DATA <speed> <battery> <temperature> <direction>` - Telemetry data
- `USERS <list>` - List of connected users
- `AUTH_SUCCESS` - Authentication successful
- `AUTH_FAILED` - Authentication failed

## 3. Message Format

### General Structure

```
<COMMAND>: <parameters>
USER: <username>
IP: <client_ip>
PORT: <client_port>
TIMESTAMP: <timestamp>

<body>
```

### Message Examples

#### Authentication Request:

```
AUTH: admin password123
USER: admin
IP: 192.168.1.100
PORT: 12345
TIMESTAMP: 2024-01-15 10:30:45
```

#### Control Command:

```
SEND_CMD: SPEED_UP
USER: admin
IP: 192.168.1.100
PORT: 12345
TIMESTAMP: 2024-01-15 10:31:00
```

#### Battery Recharge Request:

```
RECHARGE:
USER: admin
IP: 192.168.1.100
PORT: 12345
TIMESTAMP: 2024-01-15 10:31:10
```

#### Data Request:

```
GET_DATA:
USER: observer1
IP: 192.168.1.101
PORT: 12346
TIMESTAMP: 2024-01-15 10:31:15
```

#### Data Response:

```
DATA: 45 85 23 LEFT
SERVER: telemetry_server
TIMESTAMP: 2024-01-15 10:31:16
```

#### Recharge Response:

```
OK: Battery recharged to 100%
SERVER: telemetry_server
TIMESTAMP: 2024-01-15 10:31:11
```

#### Error Response:

```
ERROR: Invalid command
SERVER: telemetry_server
TIMESTAMP: 2024-01-15 10:31:20
```

## 4. Procedure Rules

### Client States:

1. **CONNECTED** - Client connected, not authenticated
2. **AUTHENTICATED** - Client authenticated as administrator
3. **OBSERVER** - Client in observer mode
4. **DISCONNECTED** - Client disconnected

### Communication Flow:

#### For Administrators:

1. Client connects → State: CONNECTED
2. Client sends AUTH → Server validates credentials
3. If valid → State: AUTHENTICATED, response: AUTH_SUCCESS
4. If invalid → State: CONNECTED, response: AUTH_FAILED
5. Client can send control commands
6. Server responds with OK/ERROR
7. Client can request LIST_USERS

#### For Observers:

1. Client connects → State: CONNECTED
2. Client can request GET_DATA immediately → State: OBSERVER
3. Server sends data periodically every 10 seconds

### Error Handling:

- Invalid commands: ERROR response with description
- Client disconnected: Remove from user list
- Malformed message: Ignore and continue
- Timeout: Close connection

## 5. Implementation Examples

### Administrator Authentication Sequence:

```
Client → Server: AUTH admin password123
Server → Client: AUTH_SUCCESS
Client → Server: SEND_CMD SPEED_UP
Server → Client: OK Speed increased to 50 km/h
Client → Server: GET_DATA
Server → Client: DATA 50 85 23 LEFT
```

### Observer Client Sequence:

```
Client → Server: GET_DATA
Server → Client: DATA 45 85 23 LEFT
[Server sends data every 10 seconds automatically]
Server → Client: DATA 47 84 24 STRAIGHT
```

### User List Sequence:

```
Client → Server: LIST_USERS
Server → Client: USERS admin(192.168.1.100:12345) observer1(192.168.1.101:12346)
```

## 6. Technical Specifications

### Encoding:

- Plain ASCII text
- Line termination: \r\n (CRLF)
- Encoding: UTF-8

### Timeouts:

- Connection: 30 seconds
- Command: 10 seconds
- Telemetry: 10 seconds (automatic)

### Limits:

- Maximum 50 concurrent clients
- Maximum 1024 bytes per message
- Persistent credentials by IP

### Logging:

- All messages are logged with timestamp
- Format: [TIMESTAMP] [IP:PORT] [TYPE] [MESSAGE]
- Types: CONNECT, DISCONNECT, COMMAND, RESPONSE, ERROR

## 7. Dynamic Battery System

### Battery Consumption:

The system implements realistic battery consumption based on vehicle usage:

- **Base consumption**: 1% per minute when stationary
- **Additional consumption**: 0.5% per minute per 10 km/h of speed
- **Consumption examples**:
  - Stationary vehicle (0 km/h): 1% per minute
  - Vehicle at 20 km/h: 2% per minute
  - Vehicle at 50 km/h: 3.5% per minute
  - Vehicle at 100 km/h: 6% per minute

### Temperature System:

Vehicle temperature varies based on usage:

- **Heating**: Increases with speed
- **Cooling**: Decreases when stationary
- **Range**: 20°C (minimum) to 50°C (maximum)

### Recharge Command:

- **Command**: `RECHARGE:`
- **Access**: Only authenticated administrators
- **Effect**: Battery returns to 100%
- **Response**: `OK: Battery recharged to 100%`

## 8. Security Implementation

### Authentication:

- Default user: admin
- Default password: admin123
- IP-based persistent authentication
- Sessions do not expire

### Validation:

- Verify command format
- Validate numeric parameters
- Sanitize user inputs
- Speed and temperature limits

---

**Protocol Version**: 1.0  
**Date**: 04/10/2025  
**Authors**: Santiago Gómez Ospina, Isabella Camacho, Sofía Isaareth Flores.
