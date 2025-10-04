/*
 * Servidor de Telemetría para Vehículo Autónomo - Versión Refactorizada
 * Implementa un sistema cliente-servidor con soporte multicliente
 * Arquitectura modular con separación de responsabilidades
 * 
 * Compilación: make
 * Uso: ./server <port> <LogsFile>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h>

// Módulos del sistema
#include "socket_manager.h"
#include "client_manager.h"
#include "vehicle.h"
#include "logger.h"
#include "protocol.h"

// Variables globales para manejo de señales
static int running = 1;
static socket_manager_t socket_mgr;
static client_manager_t client_mgr;
static vehicle_state_t vehicle;
static logger_t logger;

// Prototipos de funciones
void* handle_client(void* arg);
void* telemetry_thread(void* arg);
void* cleanup_thread(void* arg);
void signal_handler(int sig);
void cleanup_resources(void);

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

    // Inicializar módulos
    if (logger_init(&logger, log_filename) != 0) {
        fprintf(stderr, "Error inicializando logger\n");
        exit(1);
    }

    if (socket_manager_init(&socket_mgr, port) != 0) {
        fprintf(stderr, "Error inicializando socket manager\n");
        logger_cleanup(&logger);
        exit(1);
    }

    client_manager_init(&client_mgr);
    vehicle_init(&vehicle);

    printf("Servidor iniciado en puerto %d\n", port);
    printf("Archivo de logs: %s\n", log_filename);
    logger_log(&logger, LOG_SERVER_START, "0.0.0.0", port, "Servidor iniciado");

    // Crear hilo para telemetría automática
    pthread_t telemetry_tid;
    if (pthread_create(&telemetry_tid, NULL, telemetry_thread, NULL) != 0) {
        perror("Error creando hilo de telemetría");
        cleanup_resources();
        exit(1);
    }

    // Crear hilo para limpieza de clientes inactivos
    pthread_t cleanup_tid;
    if (pthread_create(&cleanup_tid, NULL, cleanup_thread, NULL) != 0) {
        perror("Error creando hilo de limpieza");
        cleanup_resources();
        exit(1);
    }

    // Bucle principal - aceptar conexiones
    while (running) {
        struct sockaddr_in client_addr;
        int client_socket = socket_manager_accept_client(&socket_mgr, &client_addr);
        
        if (client_socket < 0) {
            if (running) {
                perror("Error aceptando conexión");
            }
            continue;
        }

        // Verificar si hay espacio para más clientes
        if (client_mgr.client_count >= MAX_CLIENTS) {
            socket_close_connection(client_socket);
            logger_log(&logger, LOG_CONNECTION_REJECTED, 
                      inet_ntoa(client_addr.sin_addr), 
                      ntohs(client_addr.sin_port), 
                      "Máximo de clientes alcanzado");
            continue;
        }

        // Agregar nuevo cliente
        int client_index = client_manager_add_client(&client_mgr, client_socket, 
                                                   inet_ntoa(client_addr.sin_addr), 
                                                   ntohs(client_addr.sin_port));
        
        if (client_index == -1) {
            socket_close_connection(client_socket);
            logger_log(&logger, LOG_CONNECTION_REJECTED, 
                      inet_ntoa(client_addr.sin_addr), 
                      ntohs(client_addr.sin_port), 
                      "Error agregando cliente");
            continue;
        }

        logger_log(&logger, LOG_CONNECT, 
                  inet_ntoa(client_addr.sin_addr), 
                  ntohs(client_addr.sin_port), 
                  "Cliente conectado");

        // Crear hilo para manejar el cliente
        pthread_t client_tid;
        if (pthread_create(&client_tid, NULL, handle_client, &client_mgr.clients[client_index]) != 0) {
            perror("Error creando hilo para cliente");
            client_manager_remove_client(&client_mgr, client_index);
            socket_close_connection(client_socket);
        } else {
            pthread_detach(client_tid);
        }
    }

    cleanup_resources();
    printf("Servidor cerrado\n");
    return 0;
}

// Hilo para manejar un cliente específico
void* handle_client(void* arg) {
    client_t* client = (client_t*)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    parsed_command_t parsed_cmd;

    while (running && client->socket != -1) {
        bytes_received = socket_receive_data(client->socket, buffer, sizeof(buffer));
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                logger_log(&logger, LOG_DISCONNECT, client->ip, client->port, "Cliente desconectado");
            } else {
                logger_log(&logger, LOG_ERROR, client->ip, client->port, "Error recibiendo datos");
            }
            break;
        }

        // Actualizar actividad del cliente
        int client_index = client_manager_find_by_socket(&client_mgr, client->socket);
        if (client_index != -1) {
            client_manager_update_activity(&client_mgr, client_index);
        }

        // Log del comando recibido
        logger_log(&logger, LOG_COMMAND, client->ip, client->port, buffer);

        // Procesar comando
        protocol_parse_command(buffer, &parsed_cmd);
        protocol_handle_command(&parsed_cmd, client->socket, &client_mgr, &vehicle, &logger);
    }

    // Remover cliente de la lista
    int client_index = client_manager_find_by_socket(&client_mgr, client->socket);
    if (client_index != -1) {
        client_manager_remove_client(&client_mgr, client_index);
    }

    socket_close_connection(client->socket);
    return NULL;
}

// Hilo para enviar telemetría automática cada 10 segundos
void* telemetry_thread(void* arg) {
    (void)arg; // Evitar warning de parámetro no usado
    while (running) {
        sleep(TELEMETRY_INTERVAL);
        if (running) {
            protocol_send_telemetry_to_all(&client_mgr, &vehicle, &logger);
        }
    }
    return NULL;
}

// Hilo para limpiar clientes inactivos
void* cleanup_thread(void* arg) {
    (void)arg; // Evitar warning de parámetro no usado
    while (running) {
        sleep(30); // Verificar cada 30 segundos
        
        client_manager_cleanup_inactive(&client_mgr);
        
        // Cerrar sockets de clientes inactivos
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_mgr.clients[i].socket == -1) {
                // El cliente ya fue marcado como inactivo, cerrar socket si es necesario
                // (esto se maneja en el hilo principal)
            }
        }
    }
    return NULL;
}

// Manejador de señales para cierre limpio
void signal_handler(int sig) {
    (void)sig; // Evitar warning de parámetro no usado
    printf("\nCerrando servidor...\n");
    running = 0;
    socket_manager_close(&socket_mgr);
}

// Limpiar recursos al salir
void cleanup_resources(void) {
    running = 0;
    
    // Cerrar todos los sockets de clientes
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_mgr.clients[i].socket != -1) {
            socket_close_connection(client_mgr.clients[i].socket);
        }
    }
    
    // Limpiar módulos
    socket_manager_close(&socket_mgr);
    client_manager_cleanup(&client_mgr);
    vehicle_cleanup(&vehicle);
    logger_cleanup(&logger);
}
