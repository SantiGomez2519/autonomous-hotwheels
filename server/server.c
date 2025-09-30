/*
 * Servidor de Telemetría para Vehículo Autónomo
 * Implementa un sistema cliente-servidor con soporte multicliente
 * Protocolo personalizado para control y telemetría
 * 
 * Compilación: gcc -o server server.c -lpthread
 * Uso: ./server <port> <LogsFile>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

// Constantes del sistema
#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define TELEMETRY_INTERVAL 10

// Estructura para representar un cliente conectado
typedef struct {
    int socket;
    char ip[INET_ADDRSTRLEN];
    int port;
    char username[MAX_USERNAME];
    int is_admin;
    int authenticated;
    time_t last_activity;
} client_t;

// Estructura para el estado del vehículo
typedef struct {
    int speed;          // km/h
    int battery;        // porcentaje
    int temperature;    // grados celsius
    char direction[20]; // LEFT, RIGHT, STRAIGHT
} vehicle_state_t;

// Variables globales
client_t clients[MAX_CLIENTS];
int client_count = 0;
vehicle_state_t vehicle;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vehicle_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_socket;
FILE *log_file;
int running = 1;

// Credenciales por defecto
const char* DEFAULT_USERNAME = "admin";
const char* DEFAULT_PASSWORD = "admin123";

// Prototipos de funciones
void* handle_client(void* arg);
void* telemetry_thread(void* arg);
void* cleanup_thread(void* arg);
void send_telemetry_to_all();
void log_message(const char* type, const char* ip, int port, const char* message);
void remove_client(int client_index);
int find_client_by_socket(int socket);
void parse_command(int client_socket, const char* command);
void send_response(int socket, const char* response);
void signal_handler(int sig);

// Función principal
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Uso: %s <port> <LogsFile>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    char* log_filename = argv[2];

    // Configurar manejador de señales
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Abrir archivo de logs
    log_file = fopen(log_filename, "a");
    if (!log_file) {
        perror("Error abriendo archivo de logs");
        exit(1);
    }

    // Inicializar estado del vehículo
    pthread_mutex_lock(&vehicle_mutex);
    vehicle.speed = 0;
    vehicle.battery = 100;
    vehicle.temperature = 20;
    strcpy(vehicle.direction, "STRAIGHT");
    pthread_mutex_unlock(&vehicle_mutex);

    // Inicializar array de clientes
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = -1;
        clients[i].authenticated = 0;
        clients[i].is_admin = 0;
    }

    // Crear socket del servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creando socket");
        exit(1);
    }

    // Configurar opciones del socket
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error configurando socket");
        exit(1);
    }

    // Configurar dirección del servidor
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Vincular socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error vinculando socket");
        exit(1);
    }

    // Escuchar conexiones
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Error escuchando conexiones");
        exit(1);
    }

    printf("Servidor iniciado en puerto %d\n", port);
    printf("Archivo de logs: %s\n", log_filename);
    log_message("SERVER_START", "0.0.0.0", port, "Servidor iniciado");

    // Crear hilo para telemetría automática
    pthread_t telemetry_tid;
    if (pthread_create(&telemetry_tid, NULL, telemetry_thread, NULL) != 0) {
        perror("Error creando hilo de telemetría");
        exit(1);
    }

    // Crear hilo para limpieza de clientes inactivos
    pthread_t cleanup_tid;
    if (pthread_create(&cleanup_tid, NULL, cleanup_thread, NULL) != 0) {
        perror("Error creando hilo de limpieza");
        exit(1);
    }

    // Bucle principal - aceptar conexiones
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running) {
                perror("Error aceptando conexión");
            }
            continue;
        }

        // Verificar si hay espacio para más clientes
        pthread_mutex_lock(&clients_mutex);
        if (client_count >= MAX_CLIENTS) {
            close(client_socket);
            pthread_mutex_unlock(&clients_mutex);
            log_message("CONNECTION_REJECTED", inet_ntoa(client_addr.sin_addr), 
                       ntohs(client_addr.sin_port), "Máximo de clientes alcanzado");
            continue;
        }

        // Agregar nuevo cliente
        int client_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket == -1) {
                client_index = i;
                break;
            }
        }

        if (client_index != -1) {
            clients[client_index].socket = client_socket;
            strcpy(clients[client_index].ip, inet_ntoa(client_addr.sin_addr));
            clients[client_index].port = ntohs(client_addr.sin_port);
            clients[client_index].authenticated = 0;
            clients[client_index].is_admin = 0;
            clients[client_index].last_activity = time(NULL);
            strcpy(clients[client_index].username, "");
            client_count++;

            log_message("CONNECT", clients[client_index].ip, 
                       clients[client_index].port, "Cliente conectado");

            // Crear hilo para manejar el cliente
            pthread_t client_tid;
            if (pthread_create(&client_tid, NULL, handle_client, &clients[client_index]) != 0) {
                perror("Error creando hilo para cliente");
                remove_client(client_index);
            } else {
                pthread_detach(client_tid);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    // Limpiar recursos
    close(server_socket);
    fclose(log_file);
    printf("Servidor cerrado\n");
    return 0;
}

// Hilo para manejar un cliente específico
void* handle_client(void* arg) {
    client_t* client = (client_t*)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while (running && client->socket != -1) {
        bytes_received = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                log_message("DISCONNECT", client->ip, client->port, "Cliente desconectado");
            } else {
                log_message("ERROR", client->ip, client->port, "Error recibiendo datos");
            }
            break;
        }

        buffer[bytes_received] = '\0';
        client->last_activity = time(NULL);

        // Log del comando recibido
        log_message("COMMAND", client->ip, client->port, buffer);

        // Procesar comando
        parse_command(client->socket, buffer);
    }

    // Remover cliente de la lista
    pthread_mutex_lock(&clients_mutex);
    int client_index = find_client_by_socket(client->socket);
    if (client_index != -1) {
        remove_client(client_index);
    }
    pthread_mutex_unlock(&clients_mutex);

    close(client->socket);
    return NULL;
}

// Hilo para enviar telemetría automática cada 10 segundos
void* telemetry_thread(void* arg) {
    while (running) {
        sleep(TELEMETRY_INTERVAL);
        if (running) {
            send_telemetry_to_all();
        }
    }
    return NULL;
}

// Hilo para limpiar clientes inactivos
void* cleanup_thread(void* arg) {
    while (running) {
        sleep(30); // Verificar cada 30 segundos
        
        pthread_mutex_lock(&clients_mutex);
        time_t current_time = time(NULL);
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != -1 && 
                (current_time - clients[i].last_activity) > 300) { // 5 minutos de inactividad
                
                log_message("TIMEOUT", clients[i].ip, clients[i].port, "Cliente inactivo removido");
                close(clients[i].socket);
                remove_client(i);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

// Enviar datos de telemetría a todos los clientes
void send_telemetry_to_all() {
    pthread_mutex_lock(&vehicle_mutex);
    char telemetry_data[BUFFER_SIZE];
    snprintf(telemetry_data, BUFFER_SIZE, 
             "DATA: %d %d %d %s\r\nSERVER: telemetry_server\r\nTIMESTAMP: %ld\r\n\r\n",
             vehicle.speed, vehicle.battery, vehicle.temperature, 
             vehicle.direction, time(NULL));
    pthread_mutex_unlock(&vehicle_mutex);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != -1) {
            if (send(clients[i].socket, telemetry_data, strlen(telemetry_data), 0) < 0) {
                // Cliente desconectado, remover
                close(clients[i].socket);
                remove_client(i);
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Registrar mensaje en archivo de logs
void log_message(const char* type, const char* ip, int port, const char* message) {
    time_t now = time(NULL);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    fprintf(log_file, "[%s] [%s:%d] [%s] %s\n", timestamp, ip, port, type, message);
    fflush(log_file);
    
    // También imprimir en consola
    printf("[%s] [%s:%d] [%s] %s\n", timestamp, ip, port, type, message);
}

// Remover cliente de la lista
void remove_client(int client_index) {
    if (client_index >= 0 && client_index < MAX_CLIENTS) {
        if (clients[client_index].socket != -1) {
            clients[client_index].socket = -1;
            client_count--;
        }
    }
}

// Buscar cliente por socket
int find_client_by_socket(int socket) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == socket) {
            return i;
        }
    }
    return -1;
}

// Procesar comando recibido del cliente
void parse_command(int client_socket, const char* command) {
    char response[BUFFER_SIZE];
    char cmd[100], param1[100], param2[100];
    
    // Parsear comando básico
    if (sscanf(command, "AUTH: %s %s", param1, param2) == 2) {
        // Comando de autenticación
        if (strcmp(param1, DEFAULT_USERNAME) == 0 && strcmp(param2, DEFAULT_PASSWORD) == 0) {
            // Autenticación exitosa
            pthread_mutex_lock(&clients_mutex);
            int client_index = find_client_by_socket(client_socket);
            if (client_index != -1) {
                clients[client_index].authenticated = 1;
                clients[client_index].is_admin = 1;
                strcpy(clients[client_index].username, param1);
            }
            pthread_mutex_unlock(&clients_mutex);
            
            strcpy(response, "AUTH_SUCCESS\r\n\r\n");
            log_message("AUTH_SUCCESS", "", 0, param1);
        } else {
            strcpy(response, "AUTH_FAILED\r\n\r\n");
            log_message("AUTH_FAILED", "", 0, param1);
        }
        send_response(client_socket, response);
        return;
    }
    
    if (strstr(command, "GET_DATA:") != NULL) {
        // Solicitud de datos de telemetría
        pthread_mutex_lock(&vehicle_mutex);
        snprintf(response, BUFFER_SIZE, 
                "DATA: %d %d %d %s\r\nSERVER: telemetry_server\r\nTIMESTAMP: %ld\r\n\r\n",
                vehicle.speed, vehicle.battery, vehicle.temperature, 
                vehicle.direction, time(NULL));
        pthread_mutex_unlock(&vehicle_mutex);
        
        send_response(client_socket, response);
        log_message("DATA_SENT", "", 0, "Datos de telemetría enviados");
        return;
    }
    
    if (sscanf(command, "SEND_CMD: %s", cmd) == 1) {
        // Comando de control del vehículo
        pthread_mutex_lock(&clients_mutex);
        int client_index = find_client_by_socket(client_socket);
        if (client_index == -1 || !clients[client_index].is_admin) {
            pthread_mutex_unlock(&clients_mutex);
            strcpy(response, "ERROR: No autorizado\r\n\r\n");
            send_response(client_socket, response);
            log_message("UNAUTHORIZED", "", 0, "Intento de comando sin autorización");
            return;
        }
        pthread_mutex_unlock(&clients_mutex);
        
        // Procesar comando de control
        pthread_mutex_lock(&vehicle_mutex);
        if (strcmp(cmd, "SPEED_UP") == 0) {
            if (vehicle.speed < 100) {
                vehicle.speed += 10;
                snprintf(response, BUFFER_SIZE, "OK: Velocidad aumentada a %d km/h\r\n\r\n", vehicle.speed);
            } else {
                strcpy(response, "ERROR: Velocidad máxima alcanzada\r\n\r\n");
            }
        } else if (strcmp(cmd, "SLOW_DOWN") == 0) {
            if (vehicle.speed > 0) {
                vehicle.speed -= 10;
                snprintf(response, BUFFER_SIZE, "OK: Velocidad reducida a %d km/h\r\n\r\n", vehicle.speed);
            } else {
                strcpy(response, "ERROR: Velocidad mínima alcanzada\r\n\r\n");
            }
        } else if (strcmp(cmd, "TURN_LEFT") == 0) {
            strcpy(vehicle.direction, "LEFT");
            strcpy(response, "OK: Girando a la izquierda\r\n\r\n");
        } else if (strcmp(cmd, "TURN_RIGHT") == 0) {
            strcpy(vehicle.direction, "RIGHT");
            strcpy(response, "OK: Girando a la derecha\r\n\r\n");
        } else {
            strcpy(response, "ERROR: Comando no válido\r\n\r\n");
        }
        pthread_mutex_unlock(&vehicle_mutex);
        
        send_response(client_socket, response);
        log_message("COMMAND_EXECUTED", "", 0, cmd);
        return;
    }
    
    if (strstr(command, "LIST_USERS:") != NULL) {
        // Listar usuarios conectados
        pthread_mutex_lock(&clients_mutex);
        int client_index = find_client_by_socket(client_socket);
        if (client_index == -1 || !clients[client_index].is_admin) {
            pthread_mutex_unlock(&clients_mutex);
            strcpy(response, "ERROR: No autorizado\r\n\r\n");
            send_response(client_socket, response);
            return;
        }
        
        strcpy(response, "USERS: ");
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != -1) {
                char user_info[200];
                snprintf(user_info, sizeof(user_info), "%s(%s:%d) ", 
                        clients[i].username, clients[i].ip, clients[i].port);
                strcat(response, user_info);
            }
        }
        strcat(response, "\r\n\r\n");
        pthread_mutex_unlock(&clients_mutex);
        
        send_response(client_socket, response);
        log_message("USERS_LIST", "", 0, "Lista de usuarios enviada");
        return;
    }
    
    if (strstr(command, "DISCONNECT:") != NULL) {
        strcpy(response, "OK: Desconectando\r\n\r\n");
        send_response(client_socket, response);
        log_message("DISCONNECT_REQUEST", "", 0, "Solicitud de desconexión");
        return;
    }
    
    // Comando no reconocido
    strcpy(response, "ERROR: Comando no reconocido\r\n\r\n");
    send_response(client_socket, response);
    log_message("UNKNOWN_COMMAND", "", 0, command);
}

// Enviar respuesta al cliente
void send_response(int socket, const char* response) {
    if (send(socket, response, strlen(response), 0) < 0) {
        perror("Error enviando respuesta");
    }
    log_message("RESPONSE", "", 0, response);
}

// Manejador de señales para cierre limpio
void signal_handler(int sig) {
    printf("\nCerrando servidor...\n");
    running = 0;
    close(server_socket);
}
