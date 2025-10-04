#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "client_manager.h"
#include "vehicle.h"
#include "logger.h"

// Protocol constants
#define DEFAULT_USERNAME "admin"
#define DEFAULT_PASSWORD "admin123"
#define TELEMETRY_INTERVAL 10

// Command types
typedef enum {
    CMD_AUTH,
    CMD_GET_DATA,
    CMD_SEND_CMD,
    CMD_LIST_USERS,
    CMD_DISCONNECT,
    CMD_UNKNOWN
} command_type_t;

// Structure for parsed command
typedef struct {
    command_type_t type;
    char param1[100];
    char param2[100];
    char param3[100];
} parsed_command_t;

// Protocol functions
command_type_t protocol_parse_command(const char* command, parsed_command_t* parsed);
void protocol_handle_command(parsed_command_t* cmd, int client_socket, 
                            client_manager_t* client_mgr, vehicle_state_t* vehicle, 
                            logger_t* logger);
void protocol_send_response(int socket, const char* response, logger_t* logger);
void protocol_send_telemetry_to_all(client_manager_t* client_mgr, vehicle_state_t* vehicle, logger_t* logger);

// Helper functions
const char* protocol_command_type_to_string(command_type_t type);
int protocol_validate_vehicle_command(const char* command);

#endif // PROTOCOL_H
