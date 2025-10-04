#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <sys/socket.h>
#include <netinet/in.h>

// Network constants
#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024

// Structure for socket information
typedef struct {
    int server_socket;
    int port;
    struct sockaddr_in server_addr;
} socket_manager_t;

// Socket management functions
int socket_manager_init(socket_manager_t* manager, int port);
int socket_manager_accept_client(socket_manager_t* manager, struct sockaddr_in* client_addr);
void socket_manager_close(socket_manager_t* manager);
int socket_send_data(int socket, const char* data, size_t length);
int socket_receive_data(int socket, char* buffer, size_t buffer_size);
void socket_close_connection(int socket);

#endif // SOCKET_MANAGER_H
