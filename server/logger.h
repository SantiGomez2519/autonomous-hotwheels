#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

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

// Logger structure
typedef struct {
    FILE* log_file;
    char* filename;
} logger_t;

// Logging functions
int logger_init(logger_t* logger, const char* filename);
void logger_cleanup(logger_t* logger);
void logger_log(logger_t* logger, log_type_t type, const char* ip, int port, const char* message);
void logger_log_simple(logger_t* logger, log_type_t type, const char* message);

// Helper functions for logging
const char* logger_type_to_string(log_type_t type);
void logger_print_timestamp(FILE* file);

#endif // LOGGER_H
