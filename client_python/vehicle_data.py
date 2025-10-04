#!/usr/bin/env python3
"""
Vehicle data model
Contains the current vehicle state and methods to update it
"""

class VehicleData:
    def __init__(self):
        self.speed = 0
        self.battery = 100
        self.temperature = 20
        self.direction = 'STRAIGHT'
    
    def update_from_server_data(self, parts):
        """Update data from server response"""
        if len(parts) >= 4:
            try:
                self.speed = max(0, min(100, int(parts[1])))
                self.battery = max(0, min(100, int(parts[2])))
                self.temperature = int(parts[3])
                self.direction = parts[4] if len(parts) > 4 else 'STRAIGHT'
            except (ValueError, IndexError):
                # Keep current values if there's a parsing error
                pass
    
    def get_speed_display(self):
        return f"{self.speed} km/h"
    
    def get_battery_display(self):
        return f"{self.battery}%"
    
    def get_temperature_display(self):
        return f"{self.temperature}°C"
    
    def get_direction_display(self):
        return self.direction
    
    def to_dict(self):
        """Convertir a diccionario para compatibilidad"""
        return {
            'speed': self.speed,
            'battery': self.battery,
            'temperature': self.temperature,
            'direction': self.direction
        }
