#include "socket_manager.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int socket_manager_init(socket_manager_t* manager, int port) {
    if (!manager) return -1;
    
    // Crear socket del servidor
    manager->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (manager->server_socket < 0) {
        perror("Error creando socket");
        return -1;
    }
    
    // Configurar opciones del socket
    int opt = 1;
    if (setsockopt(manager->server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error configurando socket");
        close(manager->server_socket);
        return -1;
    }
    
    // Configurar dirección del servidor
    manager->server_addr.sin_family = AF_INET;
    manager->server_addr.sin_addr.s_addr = INADDR_ANY;
    manager->server_addr.sin_port = htons(port);
    manager->port = port;
    
    // Vincular socket
    if (bind(manager->server_socket, (struct sockaddr*)&manager->server_addr, sizeof(manager->server_addr)) < 0) {
        perror("Error vinculando socket");
        close(manager->server_socket);
        return -1;
    }
    
    // Escuchar conexiones
    if (listen(manager->server_socket, MAX_CLIENTS) < 0) {
        perror("Error escuchando conexiones");
        close(manager->server_socket);
        return -1;
    }
    
    return 0;
}

int socket_manager_accept_client(socket_manager_t* manager, struct sockaddr_in* client_addr) {
    if (!manager || !client_addr) return -1;
    
    socklen_t client_len = sizeof(*client_addr);
    int client_socket = accept(manager->server_socket, (struct sockaddr*)client_addr, &client_len);
    
    if (client_socket < 0) {
        if (errno != EINTR) {
            perror("Error aceptando conexión");
        }
        return -1;
    }
    
    return client_socket;
}

void socket_manager_close(socket_manager_t* manager) {
    if (manager && manager->server_socket >= 0) {
        close(manager->server_socket);
        manager->server_socket = -1;
    }
}

int socket_send_data(int socket, const char* data, size_t length) {
    if (socket < 0 || !data) return -1;
    
    ssize_t bytes_sent = send(socket, data, length, 0);
    if (bytes_sent < 0) {
        perror("Error enviando datos");
        return -1;
    }
    
    return (int)bytes_sent;
}

int socket_receive_data(int socket, char* buffer, size_t buffer_size) {
    if (socket < 0 || !buffer || buffer_size == 0) return -1;
    
    ssize_t bytes_received = recv(socket, buffer, buffer_size - 1, 0);
    if (bytes_received < 0) {
        perror("Error recibiendo datos");
        return -1;
    }
    
    buffer[bytes_received] = '\0';
    return (int)bytes_received;
}

void socket_close_connection(int socket) {
    if (socket >= 0) {
        close(socket);
    }
}
