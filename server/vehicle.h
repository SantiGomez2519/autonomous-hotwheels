#ifndef VEHICLE_H
#define VEHICLE_H

#include <pthread.h>

// Structure for vehicle state
typedef struct {
    int speed;          // km/h (0-100)
    int battery;        // percentage (0-100)
    int temperature;    // celsius degrees
    char direction[20]; // LEFT, RIGHT, STRAIGHT
    time_t last_update; // timestamp of last battery update
    pthread_mutex_t mutex;
} vehicle_state_t;

// Vehicle management functions
void vehicle_init(vehicle_state_t* vehicle);
void vehicle_cleanup(vehicle_state_t* vehicle);
void vehicle_get_state(vehicle_state_t* vehicle, int* speed, int* battery, int* temperature, char* direction);
void vehicle_set_speed(vehicle_state_t* vehicle, int speed);
void vehicle_set_direction(vehicle_state_t* vehicle, const char* direction);
int vehicle_speed_up(vehicle_state_t* vehicle);
int vehicle_slow_down(vehicle_state_t* vehicle);
void vehicle_update_battery(vehicle_state_t* vehicle);
void vehicle_recharge_battery(vehicle_state_t* vehicle);
void vehicle_format_telemetry(vehicle_state_t* vehicle, char* buffer, size_t buffer_size);

#endif // VEHICLE_H
