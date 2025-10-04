#include "protocol.h"
#include <string.h>
#include <stdio.h>

command_type_t protocol_parse_command(const char* command, parsed_command_t* parsed) {
    if (!command || !parsed) return CMD_UNKNOWN;
    
    // Clear structure
    memset(parsed, 0, sizeof(parsed_command_t));
    
    // Parse authentication command
    if (sscanf(command, "AUTH: %s %s", parsed->param1, parsed->param2) == 2) {
        parsed->type = CMD_AUTH;
        return CMD_AUTH;
    }
    
    // Parse data request
    if (strstr(command, "GET_DATA:") != NULL) {
        parsed->type = CMD_GET_DATA;
        return CMD_GET_DATA;
    }
    
    // Parse vehicle control command
    if (sscanf(command, "SEND_CMD: %s", parsed->param1) == 1) {
        parsed->type = CMD_SEND_CMD;
        return CMD_SEND_CMD;
    }
    
    // Parse user list request
    if (strstr(command, "LIST_USERS:") != NULL) {
        parsed->type = CMD_LIST_USERS;
        return CMD_LIST_USERS;
    }
    
    // Parse disconnect request
    if (strstr(command, "DISCONNECT:") != NULL) {
        parsed->type = CMD_DISCONNECT;
        return CMD_DISCONNECT;
    }
    
    parsed->type = CMD_UNKNOWN;
    return CMD_UNKNOWN;
}

void protocol_handle_command(parsed_command_t* cmd, int client_socket, 
                            client_manager_t* client_mgr, vehicle_state_t* vehicle, 
                            logger_t* logger) {
    if (!cmd || client_socket < 0 || !client_mgr || !vehicle || !logger) return;
    
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
                logger_log_simple(logger, LOG_UNAUTHORIZED, "Intento de comando sin autorización");
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
            
            logger_log_simple(logger, LOG_COMMAND_EXECUTED, cmd->param1);
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
    if (socket < 0 || !response || !logger) return;
    
    if (send(socket, response, strlen(response), 0) < 0) {
        perror("Error sending response");
    }
    logger_log(logger, LOG_RESPONSE, "", 0, response);
}

void protocol_send_telemetry_to_all(client_manager_t* client_mgr, vehicle_state_t* vehicle, logger_t* logger) {
    if (!client_mgr || !vehicle || !logger) return;
    
    char telemetry_data[BUFFER_SIZE];
    vehicle_format_telemetry(vehicle, telemetry_data, sizeof(telemetry_data));
    
    client_manager_send_to_all(client_mgr, telemetry_data);
}

const char* protocol_command_type_to_string(command_type_t type) {
    switch (type) {
        case CMD_AUTH: return "AUTH";
        case CMD_GET_DATA: return "GET_DATA";
        case CMD_SEND_CMD: return "SEND_CMD";
        case CMD_LIST_USERS: return "LIST_USERS";
        case CMD_DISCONNECT: return "DISCONNECT";
        case CMD_UNKNOWN: return "UNKNOWN";
        default: return "INVALID";
    }
}

int protocol_validate_vehicle_command(const char* command) {
    if (!command) return 0;
    
    return (strcmp(command, "SPEED_UP") == 0 ||
            strcmp(command, "SLOW_DOWN") == 0 ||
            strcmp(command, "TURN_LEFT") == 0 ||
            strcmp(command, "TURN_RIGHT") == 0);
}
