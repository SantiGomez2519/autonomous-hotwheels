/*
 * Autonomous Vehicle Telemetry Server
 * Implements a client-server system with multi-client support
 * Modular architecture with separation of responsibilities
 * 
 * Compilation: make
 * Usage: ./server <port> <LogsFile>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h>

// System modules
#include "socket_manager.h"
#include "vehicle.h"
#include "client_protocol.h"

// Global variables for signal handling
static int running = 1;
static socket_manager_t socket_mgr;
static client_manager_t client_mgr;
static vehicle_state_t vehicle;
static logger_t logger;

// Function prototypes
void* handle_client(void* arg);
void* telemetry_thread(void* arg);
void* cleanup_thread(void* arg);
void signal_handler(int sig);
void cleanup_resources(void);

// Main function
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <port> <LogsFile>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    char* log_filename = argv[2];

    // Configure signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize modules
    if (socket_manager_init(&socket_mgr, port) != 0) {
        fprintf(stderr, "Error initializing socket manager\n");
        exit(1);
    }

    client_protocol_init(&client_mgr, &logger, log_filename);
    vehicle_init(&vehicle);

    printf("Server started on port %d\n", port);
    printf("Log file: %s\n", log_filename);
    logger_log(&logger, LOG_SERVER_START, "0.0.0.0", port, "Server started");

    // Create thread for automatic telemetry
    pthread_t telemetry_tid;
    if (pthread_create(&telemetry_tid, NULL, telemetry_thread, NULL) != 0) {
        perror("Error creating telemetry thread");
        cleanup_resources();
        exit(1);
    }

    // Create thread for inactive client cleanup
    pthread_t cleanup_tid;
    if (pthread_create(&cleanup_tid, NULL, cleanup_thread, NULL) != 0) {
        perror("Error creating cleanup thread");
        cleanup_resources();
        exit(1);
    }

    // Main loop - accept connections
    while (running) {
        struct sockaddr_in client_addr;
        int client_socket = socket_manager_accept_client(&socket_mgr, &client_addr);
        
        if (client_socket < 0) {
            if (running) {
                perror("Error accepting connection");
            }
            continue;
        }

        // Check if there is space for more clients
        if (client_mgr.client_count >= MAX_CLIENTS) {
            socket_close_connection(client_socket);
            logger_log(&logger, LOG_CONNECTION_REJECTED, 
                      inet_ntoa(client_addr.sin_addr), 
                      ntohs(client_addr.sin_port), 
                      "Maximum clients reached");
            continue;
        }

        // Add new client
        int client_index = client_manager_add_client(&client_mgr, client_socket, 
                                                   inet_ntoa(client_addr.sin_addr), 
                                                   ntohs(client_addr.sin_port));
        
        if (client_index == -1) {
            socket_close_connection(client_socket);
            logger_log(&logger, LOG_CONNECTION_REJECTED, 
                      inet_ntoa(client_addr.sin_addr), 
                      ntohs(client_addr.sin_port), 
                      "Error adding client");
            continue;
        }

        logger_log(&logger, LOG_CONNECT, 
                  inet_ntoa(client_addr.sin_addr), 
                  ntohs(client_addr.sin_port), 
                  "Client connected");

        // Create thread to handle client
        pthread_t client_tid;
        if (pthread_create(&client_tid, NULL, handle_client, &client_mgr.clients[client_index]) != 0) {
            perror("Error creating thread for client");
            client_manager_remove_client(&client_mgr, client_index);
            socket_close_connection(client_socket);
        } else {
            pthread_detach(client_tid);
        }
    }

    cleanup_resources();
    printf("Server closed\n");
    return 0;
}

// Thread to handle a specific client
void* handle_client(void* arg) {
    client_t* client = (client_t*)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    parsed_command_t parsed_cmd;

    while (running && client->socket != -1) {
        bytes_received = socket_receive_data(client->socket, buffer, sizeof(buffer));
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                logger_log(&logger, LOG_DISCONNECT, client->ip, client->port, "Client disconnected");
            } else {
                logger_log(&logger, LOG_ERROR, client->ip, client->port, "Error receiving data");
            }
            break;
        }

        // Update client activity
        int client_index = client_manager_find_by_socket(&client_mgr, client->socket);
        if (client_index != -1) {
            client_manager_update_activity(&client_mgr, client_index);
        }

        // Log received command
        logger_log(&logger, LOG_COMMAND, client->ip, client->port, buffer);

        // Process command
        protocol_parse_command(buffer, &parsed_cmd);
        protocol_handle_command(&parsed_cmd, client->socket, &client_mgr, &vehicle, &logger);
    }

    // Remove client from list
    int client_index = client_manager_find_by_socket(&client_mgr, client->socket);
    if (client_index != -1) {
        client_manager_remove_client(&client_mgr, client_index);
    }

    socket_close_connection(client->socket);
    return NULL;
}

// Thread to send automatic telemetry every 10 seconds
void* telemetry_thread(void* arg) {
    (void)arg; // Avoid unused parameter warning
    while (running) {
        sleep(TELEMETRY_INTERVAL);
        if (running) {
            protocol_send_telemetry_to_all(&client_mgr, &vehicle, &logger);
        }
    }
    return NULL;
}

// Thread to clean up inactive clients
void* cleanup_thread(void* arg) {
    (void)arg; // Avoid unused parameter warning
    while (running) {
        sleep(30); // Check every 30 seconds
        
        client_manager_cleanup_inactive(&client_mgr);
        
        // Close inactive client sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_mgr.clients[i].socket == -1) {
                // Client already marked as inactive, close socket if necessary
                // (this is handled in the main thread)
            }
        }
    }
    return NULL;
}

// Signal handler for clean shutdown
void signal_handler(int sig) {
    (void)sig; // Avoid unused parameter warning
    printf("\nClosing server...\n");
    running = 0;
    socket_manager_close(&socket_mgr);
}

// Clean up resources on exit
void cleanup_resources(void) {
    running = 0;
    
    // Close all client sockets
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_mgr.clients[i].socket != -1) {
            socket_close_connection(client_mgr.clients[i].socket);
        }
    }
    
    // Clean up modules
    socket_manager_close(&socket_mgr);
    client_protocol_cleanup(&client_mgr, &logger);
    vehicle_cleanup(&vehicle);
}
