#include "client_manager.h"
#include "protocol.h"
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

void client_manager_init(client_manager_t* manager) {
    if (!manager) return;
    
    manager->client_count = 0;
    
    // Inicializar array de clientes
    for (int i = 0; i < MAX_CLIENTS; i++) {
        manager->clients[i].socket = -1;
        manager->clients[i].authenticated = 0;
        manager->clients[i].is_admin = 0;
        manager->clients[i].username[0] = '\0';
    }
    
    if (pthread_mutex_init(&manager->mutex, NULL) != 0) {
        perror("Error inicializando mutex del gestor de clientes");
    }
}

void client_manager_cleanup(client_manager_t* manager) {
    if (manager) {
        pthread_mutex_destroy(&manager->mutex);
    }
}

int client_manager_add_client(client_manager_t* manager, int socket, const char* ip, int port) {
    if (!manager || socket < 0 || !ip) return -1;
    
    pthread_mutex_lock(&manager->mutex);
    
    if (manager->client_count >= MAX_CLIENTS) {
        pthread_mutex_unlock(&manager->mutex);
        return -1; // No hay espacio
    }
    
    // Buscar slot vacío
    int client_index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (manager->clients[i].socket == -1) {
            client_index = i;
            break;
        }
    }
    
    if (client_index == -1) {
        pthread_mutex_unlock(&manager->mutex);
        return -1; // No hay slots disponibles
    }
    
    // Configurar nuevo cliente
    manager->clients[client_index].socket = socket;
    strncpy(manager->clients[client_index].ip, ip, INET_ADDRSTRLEN - 1);
    manager->clients[client_index].ip[INET_ADDRSTRLEN - 1] = '\0';
    manager->clients[client_index].port = port;
    manager->clients[client_index].authenticated = 0;
    manager->clients[client_index].is_admin = 0;
    manager->clients[client_index].last_activity = time(NULL);
    manager->clients[client_index].username[0] = '\0';
    
    manager->client_count++;
    
    pthread_mutex_unlock(&manager->mutex);
    return client_index;
}

void client_manager_remove_client(client_manager_t* manager, int client_index) {
    if (!manager || client_index < 0 || client_index >= MAX_CLIENTS) return;
    
    pthread_mutex_lock(&manager->mutex);
    
    if (manager->clients[client_index].socket != -1) {
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
    manager->clients[client_index].last_activity = time(NULL);
    pthread_mutex_unlock(&manager->mutex);
}

void client_manager_cleanup_inactive(client_manager_t* manager) {
    if (!manager) return;
    
    pthread_mutex_lock(&manager->mutex);
    
    time_t current_time = time(NULL);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (manager->clients[i].socket != -1 && 
            (current_time - manager->clients[i].last_activity) > CLIENT_TIMEOUT_SECONDS) {
            
            // Marcar para remoción (se cerrará el socket externamente)
            manager->clients[i].socket = -1;
            manager->clients[i].authenticated = 0;
            manager->clients[i].is_admin = 0;
            manager->clients[i].username[0] = '\0';
            manager->client_count--;
        }
    }
    
    pthread_mutex_unlock(&manager->mutex);
}

void client_manager_send_to_all(client_manager_t* manager, const char* data) {
    if (!manager || !data) return;
    
    pthread_mutex_lock(&manager->mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (manager->clients[i].socket != -1) {
            if (send(manager->clients[i].socket, data, strlen(data), 0) < 0) {
                // Cliente desconectado, marcar para remoción
                manager->clients[i].socket = -1;
                manager->clients[i].authenticated = 0;
                manager->clients[i].is_admin = 0;
                manager->clients[i].username[0] = '\0';
                manager->client_count--;
            }
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
    
    // Verificar credenciales
    if (strcmp(username, DEFAULT_USERNAME) == 0 && strcmp(password, DEFAULT_PASSWORD) == 0) {
        pthread_mutex_lock(&manager->mutex);
        
        if (manager->clients[client_index].socket != -1) {
            manager->clients[client_index].authenticated = 1;
            manager->clients[client_index].is_admin = 1;
            strncpy(manager->clients[client_index].username, username, MAX_USERNAME - 1);
            manager->clients[client_index].username[MAX_USERNAME - 1] = '\0';
        }
        
        pthread_mutex_unlock(&manager->mutex);
        return 1; // Autenticación exitosa
    }
    
    return 0; // Autenticación fallida
}
