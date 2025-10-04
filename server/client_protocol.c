#include "client_protocol.h"
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

// Authentication constants (defined here to avoid circular dependencies)
#define DEFAULT_USERNAME "admin"
#define DEFAULT_PASSWORD "admin123"

// ============================================================================
// INITIALIZATION AND CLEANUP
// ============================================================================

void client_protocol_init(client_manager_t* manager, logger_t* logger, const char* log_filename) {
    if (!manager || !logger) return;
    
    // Initialize client manager
    manager->client_count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        manager->clients[i].socket = -1;
        manager->clients[i].authenticated = 0;
        manager->clients[i].is_admin = 0;
        manager->clients[i].username[0] = '\0';
    }
    
    if (pthread_mutex_init(&manager->mutex, NULL) != 0) {
        perror("Error initializing client manager mutex");
    }
    
    // Initialize logger
    logger->log_file = fopen(log_filename, "a");
    if (!logger->log_file) {
        perror("Error opening log file");
        return;
    }
    
    // Duplicate filename
    logger->filename = malloc(strlen(log_filename) + 1);
    if (logger->filename) {
        strcpy(logger->filename, log_filename);
    }
}

void client_protocol_cleanup(client_manager_t* manager, logger_t* logger) {
    if (manager) {
        pthread_mutex_destroy(&manager->mutex);
    }
    
    if (logger) {
        if (logger->log_file) {
            fclose(logger->log_file);
        }
        if (logger->filename) {
            free(logger->filename);
        }
    }
}

// ============================================================================
// CLIENT MANAGEMENT FUNCTIONS
// ============================================================================

int client_manager_add_client(client_manager_t* manager, int socket, const char* ip, int port) {
    if (!manager || socket < 0 || !ip) return -1;
    
    pthread_mutex_lock(&manager->mutex);
    
    if (manager->client_count >= MAX_CLIENTS) {
        pthread_mutex_unlock(&manager->mutex);
        return -1; // No space available
    }
    
    // Find empty slot
    int client_index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (manager->clients[i].socket == -1) {
            client_index = i;
            break;
        }
    }
    
    if (client_index == -1) {
        pthread_mutex_unlock(&manager->mutex);
        return -1; // No slots available
    }
    
    // Configure new client
    manager->clients[client_index].socket = socket;
    strncpy(manager->clients[client_index].ip, ip, INET_ADDRSTRLEN - 1);
    manager->clients[client_index].ip[INET_ADDRSTRLEN - 1] = '\0';
    manager->clients[client_index].port = port;
    manager->clients[client_index].authenticated = 0;
    manager->clients[client_index].is_admin = 0;
    manager->clients[client_index].username[0] = '\0';
    manager->clients[client_index].last_activity = time(NULL);
    
    manager->client_count++;
    pthread_mutex_unlock(&manager->mutex);
    
    return client_index;
}

void client_manager_remove_client(client_manager_t* manager, int client_index) {
    if (!manager || client_index < 0 || client_index >= MAX_CLIENTS) return;
    
    pthread_mutex_lock(&manager->mutex);
    
    if (manager->clients[client_index].socket != -1) {
        socket_close_connection(manager->clients[client_index].socket);
        manager->clients[client_index].socket = -1;
        manager->clients[client_index].authenticated = 0;
        manager->clients[client_index].is_admin = 0;
        manager->clients[client_index].username[0] = '\0';
        manager->client_count--;
    }
    
    pthread_mutex_unlock(&manager->mutex);
}

int client_manager_find_by_socket(client_manager_t* manager, int socket) {
    if (!manager || socket < 0) return -1;
    
    pthread_mutex_lock(&manager->mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (manager->clients[i].socket == socket) {
            pthread_mutex_unlock(&manager->mutex);
            return i;
        }
    }
    
    pthread_mutex_unlock(&manager->mutex);
    return -1;
}

void client_manager_update_activity(client_manager_t* manager, int client_index) {
    if (!manager || client_index < 0 || client_index >= MAX_CLIENTS) return;
    
    pthread_mutex_lock(&manager->mutex);
    if (manager->clients[client_index].socket != -1) {
        manager->clients[client_index].last_activity = time(NULL);
    }
    pthread_mutex_unlock(&manager->mutex);
}

void client_manager_cleanup_inactive(client_manager_t* manager) {
    if (!manager) return;
    
    time_t current_time = time(NULL);
    pthread_mutex_lock(&manager->mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (manager->clients[i].socket != -1) {
            if (current_time - manager->clients[i].last_activity > CLIENT_TIMEOUT_SECONDS) {
                // Mark as inactive but don't close here (handled in main thread)
                manager->clients[i].socket = -1;
                manager->client_count--;
            }
        }
    }
    
    pthread_mutex_unlock(&manager->mutex);
}

void client_manager_send_to_all(client_manager_t* manager, const char* data) {
    if (!manager || !data) return;
    
    pthread_mutex_lock(&manager->mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (manager->clients[i].socket != -1) {
            socket_send_data(manager->clients[i].socket, data, strlen(data));
        }
    }
    
    pthread_mutex_unlock(&manager->mutex);
}

client_t* client_manager_get_client(client_manager_t* manager, int client_index) {
    if (!manager || client_index < 0 || client_index >= MAX_CLIENTS) return NULL;
    
    pthread_mutex_lock(&manager->mutex);
    client_t* client = &manager->clients[client_index];
    pthread_mutex_unlock(&manager->mutex);
    
    return client;
}

int client_manager_authenticate_client(client_manager_t* manager, int client_index, const char* username, const char* password) {
    if (!manager || client_index < 0 || client_index >= MAX_CLIENTS || !username || !password) {
        return 0;
    }
    
    // Verify credentials
    if (strcmp(username, DEFAULT_USERNAME) == 0 && strcmp(password, DEFAULT_PASSWORD) == 0) {
        pthread_mutex_lock(&manager->mutex);
        
        if (manager->clients[client_index].socket != -1) {
            manager->clients[client_index].authenticated = 1;
            manager->clients[client_index].is_admin = 1;
            strncpy(manager->clients[client_index].username, username, MAX_USERNAME - 1);
            manager->clients[client_index].username[MAX_USERNAME - 1] = '\0';
        }
        
        pthread_mutex_unlock(&manager->mutex);
        return 1; // Authentication successful
    }
    
    return 0; // Authentication failed
}

// ============================================================================
// PROTOCOL FUNCTIONS
// ============================================================================

command_type_t protocol_parse_command(const char* command, parsed_command_t* parsed) {
    if (!command || !parsed) return CMD_UNKNOWN;
    
    // Clear structure
    memset(parsed, 0, sizeof(parsed_command_t));
    
    char cmd_copy[BUFFER_SIZE];
    strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';
    
    // Parse authentication command
    if (strncmp(cmd_copy, "AUTH:", 5) == 0) {
        parsed->type = CMD_AUTH;
        sscanf(cmd_copy, "AUTH: %s %s", parsed->param1, parsed->param2);
        return CMD_AUTH;
    }
    
    // Parse data request
    if (strncmp(cmd_copy, "GET_DATA:", 9) == 0) {
        parsed->type = CMD_GET_DATA;
        return CMD_GET_DATA;
    }
    
    // Parse vehicle control command
    if (strncmp(cmd_copy, "SEND_CMD:", 9) == 0) {
        parsed->type = CMD_SEND_CMD;
        sscanf(cmd_copy, "SEND_CMD: %s", parsed->param1);
        return CMD_SEND_CMD;
    }
    
    // Parse user list request
    if (strncmp(cmd_copy, "LIST_USERS:", 11) == 0) {
        parsed->type = CMD_LIST_USERS;
        return CMD_LIST_USERS;
    }
    
    // Parse recharge request
    if (strncmp(cmd_copy, "RECHARGE:", 9) == 0) {
        parsed->type = CMD_RECHARGE;
        return CMD_RECHARGE;
    }
    
    // Parse disconnect request
    if (strncmp(cmd_copy, "DISCONNECT:", 11) == 0) {
        parsed->type = CMD_DISCONNECT;
        return CMD_DISCONNECT;
    }
    
    return CMD_UNKNOWN;
}

void protocol_handle_command(parsed_command_t* cmd, int client_socket, 
                            client_manager_t* client_mgr, vehicle_state_t* vehicle, 
                            logger_t* logger) {
    if (!cmd || !client_mgr || !vehicle || !logger) return;
    
    char response[BUFFER_SIZE];
    int client_index = client_manager_find_by_socket(client_mgr, client_socket);
    
    switch (cmd->type) {
        case CMD_AUTH: {
            if (client_index == -1) {
                strcpy(response, "ERROR: Client not found\r\n\r\n");
                break;
            }
            
            if (client_manager_authenticate_client(client_mgr, client_index, cmd->param1, cmd->param2)) {
                strcpy(response, "AUTH_SUCCESS\r\n\r\n");
                logger_log(logger, LOG_AUTH_SUCCESS, "", 0, cmd->param1);
            } else {
                strcpy(response, "AUTH_FAILED\r\n\r\n");
                logger_log(logger, LOG_AUTH_FAILED, "", 0, cmd->param1);
            }
            break;
        }
        
        case CMD_GET_DATA: {
            vehicle_format_telemetry(vehicle, response, sizeof(response));
            logger_log_simple(logger, LOG_DATA_SENT, "Telemetry data sent");
            break;
        }
        
        case CMD_SEND_CMD: {
            if (client_index == -1) {
                strcpy(response, "ERROR: Client not found\r\n\r\n");
                break;
            }
            
            client_t* client = client_manager_get_client(client_mgr, client_index);
            if (!client || !client->is_admin) {
                strcpy(response, "ERROR: Not authorized\r\n\r\n");
                break;
            }
            
            // Process vehicle control command
            if (strcmp(cmd->param1, "SPEED_UP") == 0) {
                int new_speed = vehicle_speed_up(vehicle);
                if (new_speed >= 0) {
                    snprintf(response, sizeof(response), "OK: Speed increased to %d km/h\r\n\r\n", new_speed);
                } else {
                    strcpy(response, "ERROR: Maximum speed reached\r\n\r\n");
                }
            } else if (strcmp(cmd->param1, "SLOW_DOWN") == 0) {
                int new_speed = vehicle_slow_down(vehicle);
                if (new_speed >= 0) {
                    snprintf(response, sizeof(response), "OK: Speed reduced to %d km/h\r\n\r\n", new_speed);
                } else {
                    strcpy(response, "ERROR: Minimum speed reached\r\n\r\n");
                }
            } else if (strcmp(cmd->param1, "TURN_LEFT") == 0) {
                vehicle_set_direction(vehicle, "LEFT");
                strcpy(response, "OK: Turning left\r\n\r\n");
            } else if (strcmp(cmd->param1, "TURN_RIGHT") == 0) {
                vehicle_set_direction(vehicle, "RIGHT");
                strcpy(response, "OK: Turning right\r\n\r\n");
            } else {
                strcpy(response, "ERROR: Invalid command\r\n\r\n");
            }
            break;
        }
        
        case CMD_LIST_USERS: {
            if (client_index == -1) {
                strcpy(response, "ERROR: Client not found\r\n\r\n");
                break;
            }
            
            client_t* client = client_manager_get_client(client_mgr, client_index);
            if (!client || !client->is_admin) {
                strcpy(response, "ERROR: Not authorized\r\n\r\n");
                break;
            }
            
            // Build list of connected users
            strcpy(response, "USERS: ");
            pthread_mutex_lock(&client_mgr->mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_mgr->clients[i].socket != -1) {
                    char user_info[200];
                    snprintf(user_info, sizeof(user_info), "%s(%s:%d) ", 
                            client_mgr->clients[i].username, 
                            client_mgr->clients[i].ip, 
                            client_mgr->clients[i].port);
                    strcat(response, user_info);
                }
            }
            pthread_mutex_unlock(&client_mgr->mutex);
            strcat(response, "\r\n\r\n");
            logger_log_simple(logger, LOG_USERS_LIST, "User list sent");
            break;
        }
        
        case CMD_RECHARGE: {
            if (client_index == -1) {
                strcpy(response, "ERROR: Client not found\r\n\r\n");
                break;
            }
            
            client_t* client = client_manager_get_client(client_mgr, client_index);
            if (!client || !client->is_admin) {
                strcpy(response, "ERROR: Not authorized\r\n\r\n");
                break;
            }
            
            vehicle_recharge_battery(vehicle);
            strcpy(response, "OK: Battery recharged to 100%\r\n\r\n");
            logger_log_simple(logger, LOG_COMMAND_EXECUTED, "Battery recharged");
            break;
        }
        
        case CMD_DISCONNECT: {
            strcpy(response, "OK: Disconnecting\r\n\r\n");
            logger_log_simple(logger, LOG_DISCONNECT_REQUEST, "Disconnect request");
            break;
        }
        
        case CMD_UNKNOWN:
        default: {
            strcpy(response, "ERROR: Command not recognized\r\n\r\n");
            logger_log_simple(logger, LOG_UNKNOWN_COMMAND, "Command not recognized");
            break;
        }
    }
    
    protocol_send_response(client_socket, response, logger);
}

void protocol_send_response(int socket, const char* response, logger_t* logger) {
    if (socket < 0 || !response) return;
    
    if (socket_send_data(socket, response, strlen(response)) < 0) {
        perror("Error sending response");
    }
    
    logger_log(logger, LOG_RESPONSE, "", 0, response);
}

void protocol_send_telemetry_to_all(client_manager_t* client_mgr, vehicle_state_t* vehicle, logger_t* logger) {
    if (!client_mgr || !vehicle || !logger) return;
    
    char telemetry_data[BUFFER_SIZE];
    vehicle_format_telemetry(vehicle, telemetry_data, sizeof(telemetry_data));
    
    client_manager_send_to_all(client_mgr, telemetry_data);
    logger_log_simple(logger, LOG_DATA_SENT, "Telemetry sent to all clients");
}

// ============================================================================
// LOGGING FUNCTIONS
// ============================================================================

void logger_print_timestamp(FILE* file) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(file, "[%s] ", timestamp);
}

void logger_log(logger_t* logger, log_type_t type, const char* ip, int port, const char* message) {
    if (!logger || !logger->log_file) return;
    
    // Print timestamp
    logger_print_timestamp(logger->log_file);
    
    // Print log type and message
    fprintf(logger->log_file, "[%s] ", logger_type_to_string(type));
    if (ip && strlen(ip) > 0) {
        fprintf(logger->log_file, "%s:%d - ", ip, port);
    }
    fprintf(logger->log_file, "%s\n", message);
    fflush(logger->log_file);
    
    // Also print to console
    printf("[%s] ", logger_type_to_string(type));
    if (ip && strlen(ip) > 0) {
        printf("%s:%d - ", ip, port);
    }
    printf("%s\n", message);
}

void logger_log_simple(logger_t* logger, log_type_t type, const char* message) {
    logger_log(logger, type, "", 0, message);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

const char* logger_type_to_string(log_type_t type) {
    switch (type) {
        case LOG_SERVER_START: return "SERVER_START";
        case LOG_CONNECT: return "CONNECT";
        case LOG_DISCONNECT: return "DISCONNECT";
        case LOG_AUTH_SUCCESS: return "AUTH_SUCCESS";
        case LOG_AUTH_FAILED: return "AUTH_FAILED";
        case LOG_COMMAND: return "COMMAND";
        case LOG_RESPONSE: return "RESPONSE";
        case LOG_ERROR: return "ERROR";
        case LOG_DATA_SENT: return "DATA_SENT";
        case LOG_COMMAND_EXECUTED: return "COMMAND_EXECUTED";
        case LOG_USERS_LIST: return "USERS_LIST";
        case LOG_TIMEOUT: return "TIMEOUT";
        case LOG_CONNECTION_REJECTED: return "CONNECTION_REJECTED";
        case LOG_UNKNOWN_COMMAND: return "UNKNOWN_COMMAND";
        case LOG_UNAUTHORIZED: return "UNAUTHORIZED";
        case LOG_DISCONNECT_REQUEST: return "DISCONNECT_REQUEST";
        default: return "UNKNOWN";
    }
}

const char* protocol_command_type_to_string(command_type_t type) {
    switch (type) {
        case CMD_AUTH: return "AUTH";
        case CMD_GET_DATA: return "GET_DATA";
        case CMD_SEND_CMD: return "SEND_CMD";
        case CMD_LIST_USERS: return "LIST_USERS";
        case CMD_RECHARGE: return "RECHARGE";
        case CMD_DISCONNECT: return "DISCONNECT";
        case CMD_UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

int protocol_validate_vehicle_command(const char* command) {
    if (!command) return 0;
    
    return (strcmp(command, "SPEED_UP") == 0 ||
            strcmp(command, "SLOW_DOWN") == 0 ||
            strcmp(command, "TURN_LEFT") == 0 ||
            strcmp(command, "TURN_RIGHT") == 0);
}
