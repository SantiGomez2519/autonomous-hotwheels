#include "vehicle.h"
#include <string.h>
#include <time.h>
#include <stdio.h>

void vehicle_init(vehicle_state_t* vehicle) {
    if (!vehicle) return;
    
    vehicle->speed = 0;
    vehicle->battery = 100;
    vehicle->temperature = 20;
    strcpy(vehicle->direction, "STRAIGHT");
    
    if (pthread_mutex_init(&vehicle->mutex, NULL) != 0) {
        perror("Error inicializando mutex del vehículo");
    }
}

void vehicle_cleanup(vehicle_state_t* vehicle) {
    if (vehicle) {
        pthread_mutex_destroy(&vehicle->mutex);
    }
}

void vehicle_get_state(vehicle_state_t* vehicle, int* speed, int* battery, int* temperature, char* direction) {
    if (!vehicle) return;
    
    pthread_mutex_lock(&vehicle->mutex);
    
    if (speed) *speed = vehicle->speed;
    if (battery) *battery = vehicle->battery;
    if (temperature) *temperature = vehicle->temperature;
    if (direction) strcpy(direction, vehicle->direction);
    
    pthread_mutex_unlock(&vehicle->mutex);
}

void vehicle_set_speed(vehicle_state_t* vehicle, int speed) {
    if (!vehicle) return;
    
    pthread_mutex_lock(&vehicle->mutex);
    
    if (speed >= 0 && speed <= 100) {
        vehicle->speed = speed;
    }
    
    pthread_mutex_unlock(&vehicle->mutex);
}

void vehicle_set_direction(vehicle_state_t* vehicle, const char* direction) {
    if (!vehicle || !direction) return;
    
    pthread_mutex_lock(&vehicle->mutex);
    strncpy(vehicle->direction, direction, sizeof(vehicle->direction) - 1);
    vehicle->direction[sizeof(vehicle->direction) - 1] = '\0';
    pthread_mutex_unlock(&vehicle->mutex);
}

int vehicle_speed_up(vehicle_state_t* vehicle) {
    if (!vehicle) return -1;
    
    pthread_mutex_lock(&vehicle->mutex);
    
    if (vehicle->speed < 100) {
        vehicle->speed += 10;
        if (vehicle->speed > 100) vehicle->speed = 100;
        pthread_mutex_unlock(&vehicle->mutex);
        return vehicle->speed;
    }
    
    pthread_mutex_unlock(&vehicle->mutex);
    return -1; // Velocidad máxima alcanzada
}

int vehicle_slow_down(vehicle_state_t* vehicle) {
    if (!vehicle) return -1;
    
    pthread_mutex_lock(&vehicle->mutex);
    
    if (vehicle->speed > 0) {
        vehicle->speed -= 10;
        if (vehicle->speed < 0) vehicle->speed = 0;
        pthread_mutex_unlock(&vehicle->mutex);
        return vehicle->speed;
    }
    
    pthread_mutex_unlock(&vehicle->mutex);
    return -1; // Velocidad mínima alcanzada
}

void vehicle_format_telemetry(vehicle_state_t* vehicle, char* buffer, size_t buffer_size) {
    if (!vehicle || !buffer || buffer_size == 0) return;
    
    int speed, battery, temperature;
    char direction[20];
    
    vehicle_get_state(vehicle, &speed, &battery, &temperature, direction);
    
    time_t now = time(NULL);
    snprintf(buffer, buffer_size, 
             "DATA: %d %d %d %s\r\nSERVER: telemetry_server\r\nTIMESTAMP: %ld\r\n\r\n",
             speed, battery, temperature, direction, now);
}
