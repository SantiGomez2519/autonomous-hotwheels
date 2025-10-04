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
    vehicle->last_update = time(NULL);
    
    if (pthread_mutex_init(&vehicle->mutex, NULL) != 0) {
        perror("Error initializing vehicle mutex");
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
    return -1; // Maximum speed reached
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
    return -1; // Minimum speed reached
}

void vehicle_update_battery(vehicle_state_t* vehicle) {
    if (!vehicle) return;
    
    pthread_mutex_lock(&vehicle->mutex);
    
    time_t current_time = time(NULL);
    time_t time_diff = current_time - vehicle->last_update;
    
    if (time_diff > 0) {
        // Calculate battery consumption based on speed and time
        // Base consumption: 1% per minute when stationary
        // Additional consumption: 0.5% per minute per 10 km/h of speed
        double base_consumption = (double)time_diff / 60.0; // 1% per minute
        double speed_consumption = (double)vehicle->speed * (double)time_diff / 600.0; // 0.5% per 10 km/h per minute
        
        double total_consumption = base_consumption + speed_consumption;
        
        // Update battery (minimum 0%)
        vehicle->battery -= (int)total_consumption;
        if (vehicle->battery < 0) {
            vehicle->battery = 0;
        }
        
        // Update temperature based on speed (more speed = more heat)
        if (vehicle->speed > 0) {
            vehicle->temperature += (int)(time_diff * vehicle->speed / 1000); // Gradual increase
            if (vehicle->temperature > 50) {
                vehicle->temperature = 50; // Maximum temperature
            }
        } else {
            // Cool down when stationary
            vehicle->temperature -= (int)(time_diff / 10);
            if (vehicle->temperature < 20) {
                vehicle->temperature = 20; // Minimum temperature
            }
        }
        
        vehicle->last_update = current_time;
    }
    
    pthread_mutex_unlock(&vehicle->mutex);
}

void vehicle_recharge_battery(vehicle_state_t* vehicle) {
    if (!vehicle) return;
    
    pthread_mutex_lock(&vehicle->mutex);
    vehicle->battery = 100;
    vehicle->last_update = time(NULL);
    pthread_mutex_unlock(&vehicle->mutex);
}

void vehicle_format_telemetry(vehicle_state_t* vehicle, char* buffer, size_t buffer_size) {
    if (!vehicle || !buffer || buffer_size == 0) return;
    
    // Update battery before sending telemetry
    vehicle_update_battery(vehicle);
    
    int speed, battery, temperature;
    char direction[20];
    
    vehicle_get_state(vehicle, &speed, &battery, &temperature, direction);
    
    time_t now = time(NULL);
    snprintf(buffer, buffer_size, 
             "DATA: %d %d %d %s\r\nSERVER: telemetry_server\r\nTIMESTAMP: %ld\r\n\r\n",
             speed, battery, temperature, direction, now);
}
