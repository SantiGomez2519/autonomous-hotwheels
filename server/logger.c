#include "logger.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>

int logger_init(logger_t* logger, const char* filename) {
    if (!logger || !filename) return -1;
    
    logger->log_file = fopen(filename, "a");
    if (!logger->log_file) {
        perror("Error opening log file");
        return -1;
    }
    
    // Duplicate filename
    logger->filename = malloc(strlen(filename) + 1);
    if (!logger->filename) {
        fclose(logger->log_file);
        return -1;
    }
    strcpy(logger->filename, filename);
    
    return 0;
}

void logger_cleanup(logger_t* logger) {
    if (logger) {
        if (logger->log_file) {
            fclose(logger->log_file);
            logger->log_file = NULL;
        }
        if (logger->filename) {
            free(logger->filename);
            logger->filename = NULL;
        }
    }
}

void logger_log(logger_t* logger, log_type_t type, const char* ip, int port, const char* message) {
    if (!logger || !logger->log_file || !message) return;
    
    time_t now = time(NULL);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    const char* type_str = logger_type_to_string(type);
    
    fprintf(logger->log_file, "[%s] [%s:%d] [%s] %s\n", timestamp, ip, port, type_str, message);
    fflush(logger->log_file);
    
    // Also print to console
    printf("[%s] [%s:%d] [%s] %s\n", timestamp, ip, port, type_str, message);
}

void logger_log_simple(logger_t* logger, log_type_t type, const char* message) {
    logger_log(logger, type, "", 0, message);
}

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

void logger_print_timestamp(FILE* file) {
    time_t now = time(NULL);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(file, "[%s] ", timestamp);
}
