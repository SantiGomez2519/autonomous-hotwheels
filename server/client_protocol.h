#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include <time.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include "socket_manager.h"
#include "vehicle.h"

// Client constants
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define CLIENT_TIMEOUT_SECONDS 300
#define TELEMETRY_INTERVAL 10
#define BUFFER_SIZE 1024
#define MAX_CMD_LEN 100
#define MAX_PARAM_LEN 100

// Log types
typedef enum {
    LOG_SERVER_START,
    LOG_CONNECT,
    LOG_DISCONNECT,
    LOG_AUTH_SUCCESS,
    LOG_AUTH_FAILED,
    LOG_COMMAND,
    LOG_RESPONSE,
    LOG_ERROR,
    LOG_DATA_SENT,
    LOG_COMMAND_EXECUTED,
    LOG_USERS_LIST,
    LOG_TIMEOUT,
    LOG_CONNECTION_REJECTED,
    LOG_UNKNOWN_COMMAND,
    LOG_UNAUTHORIZED,
    LOG_DISCONNECT_REQUEST
} log_type_t;

// Command types
typedef enum {
    CMD_AUTH,
    CMD_GET_DATA,
    CMD_SEND_CMD,
    CMD_LIST_USERS,
    CMD_RECHARGE,
    CMD_DISCONNECT,
    CMD_UNKNOWN
} command_type_t;

// Structure to represent a connected client
typedef struct {
    int socket;
    char ip[INET_ADDRSTRLEN];
    int port;
    char username[MAX_USERNAME];
    int is_admin;
    int authenticated;
    time_t last_activity;
} client_t;

// Structure for parsed command
typedef struct {
    command_type_t type;
    char param1[MAX_PARAM_LEN];
    char param2[MAX_PARAM_LEN];
    char param3[MAX_PARAM_LEN];
} parsed_command_t;

// Structure for client manager
typedef struct {
    client_t clients[MAX_CLIENTS];
    int client_count;
    pthread_mutex_t mutex;
} client_manager_t;

// Logger structure
typedef struct {
    FILE* log_file;
    char* filename;
} logger_t;

// Combined client, protocol and logging functions
void client_protocol_init(client_manager_t* manager, logger_t* logger, const char* log_filename);
void client_protocol_cleanup(client_manager_t* manager, logger_t* logger);

// Client management functions
int client_manager_add_client(client_manager_t* manager, int socket, const char* ip, int port);
void client_manager_remove_client(client_manager_t* manager, int client_index);
int client_manager_find_by_socket(client_manager_t* manager, int socket);
void client_manager_update_activity(client_manager_t* manager, int client_index);
void client_manager_cleanup_inactive(client_manager_t* manager);
void client_manager_send_to_all(client_manager_t* manager, const char* data);
client_t* client_manager_get_client(client_manager_t* manager, int client_index);
int client_manager_authenticate_client(client_manager_t* manager, int client_index, const char* username, const char* password);

// Protocol functions
command_type_t protocol_parse_command(const char* command, parsed_command_t* parsed);
void protocol_handle_command(parsed_command_t* cmd, int client_socket, 
                            client_manager_t* client_mgr, vehicle_state_t* vehicle, 
                            logger_t* logger);
void protocol_send_response(int socket, const char* response, logger_t* logger);
void protocol_send_telemetry_to_all(client_manager_t* client_mgr, vehicle_state_t* vehicle, logger_t* logger);

// Logging functions
void logger_log(logger_t* logger, log_type_t type, const char* ip, int port, const char* message);
void logger_log_simple(logger_t* logger, log_type_t type, const char* message);

// Helper functions
const char* logger_type_to_string(log_type_t type);
const char* protocol_command_type_to_string(command_type_t type);
int protocol_validate_vehicle_command(const char* command);

#endif // CLIENT_PROTOCOL_H
