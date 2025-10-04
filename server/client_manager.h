#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <time.h>
#include <pthread.h>
#include <netinet/in.h>
#include "socket_manager.h"

// Constantes de cliente
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define CLIENT_TIMEOUT_SECONDS 300

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

// Estructura para el gestor de clientes
typedef struct {
    client_t clients[MAX_CLIENTS];
    int client_count;
    pthread_mutex_t mutex;
} client_manager_t;

// Funciones de gestión de clientes
void client_manager_init(client_manager_t* manager);
void client_manager_cleanup(client_manager_t* manager);
int client_manager_add_client(client_manager_t* manager, int socket, const char* ip, int port);
void client_manager_remove_client(client_manager_t* manager, int client_index);
int client_manager_find_by_socket(client_manager_t* manager, int socket);
void client_manager_update_activity(client_manager_t* manager, int client_index);
void client_manager_cleanup_inactive(client_manager_t* manager);
void client_manager_send_to_all(client_manager_t* manager, const char* data);
client_t* client_manager_get_client(client_manager_t* manager, int client_index);
int client_manager_authenticate_client(client_manager_t* manager, int client_index, const char* username, const char* password);

#endif // CLIENT_MANAGER_H
