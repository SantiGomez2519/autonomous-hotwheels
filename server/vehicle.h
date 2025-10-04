#ifndef VEHICLE_H
#define VEHICLE_H

#include <pthread.h>

// Estructura para el estado del vehículo
typedef struct {
    int speed;          // km/h (0-100)
    int battery;        // porcentaje (0-100)
    int temperature;    // grados celsius
    char direction[20]; // LEFT, RIGHT, STRAIGHT
    pthread_mutex_t mutex;
} vehicle_state_t;

// Funciones de gestión del vehículo
void vehicle_init(vehicle_state_t* vehicle);
void vehicle_cleanup(vehicle_state_t* vehicle);
void vehicle_get_state(vehicle_state_t* vehicle, int* speed, int* battery, int* temperature, char* direction);
void vehicle_set_speed(vehicle_state_t* vehicle, int speed);
void vehicle_set_direction(vehicle_state_t* vehicle, const char* direction);
int vehicle_speed_up(vehicle_state_t* vehicle);
int vehicle_slow_down(vehicle_state_t* vehicle);
void vehicle_format_telemetry(vehicle_state_t* vehicle, char* buffer, size_t buffer_size);

#endif // VEHICLE_H
