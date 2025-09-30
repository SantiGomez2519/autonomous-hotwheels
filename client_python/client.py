#!/usr/bin/env python3
"""
Cliente Python para Sistema de Telemetría Vehículo Autónomo
Implementa conexión TCP con el servidor y manejo de comandos
Soporta dos tipos de usuarios: Administrador y Observador
"""

import socket
import threading
import time
import json
from datetime import datetime
from gui import TelemetryGUI

class TelemetryClient:
    def __init__(self, host='localhost', port=8080):
        self.host = host
        self.port = port
        self.socket = None
        self.connected = False
        self.authenticated = False
        self.is_admin = False
        self.username = ""
        self.running = True
        
        # Estado del vehículo
        self.vehicle_data = {
            'speed': 0,
            'battery': 100,
            'temperature': 20,
            'direction': 'STRAIGHT'
        }
        
        # Lista de usuarios conectados (solo para admins)
        self.connected_users = []
        
        # GUI
        self.gui = TelemetryGUI(self)
        
    def connect(self):
        """Conectar al servidor"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(10)  # Timeout de 10 segundos
            self.socket.connect((self.host, self.port))
            self.connected = True
            self.gui.log_message(f"Conectado al servidor {self.host}:{self.port}")
            
            # Iniciar hilo para recibir mensajes
            self.receive_thread = threading.Thread(target=self.receive_messages)
            self.receive_thread.daemon = True
            self.receive_thread.start()
            
            return True
        except Exception as e:
            self.gui.log_message(f"Error conectando: {str(e)}")
            return False
    
    def disconnect(self):
        """Desconectar del servidor"""
        try:
            if self.connected:
                self.send_command("DISCONNECT:")
                self.connected = False
                self.authenticated = False
                self.is_admin = False
                self.socket.close()
                self.gui.log_message("Desconectado del servidor")
        except Exception as e:
            self.gui.log_message(f"Error desconectando: {str(e)}")
    
    def authenticate(self, username, password):
        """Autenticar como administrador"""
        if not self.connected:
            return False
        
        try:
            command = f"AUTH: {username} {password}"
            self.send_command(command)
            # La respuesta se procesará en receive_messages
            return True
        except Exception as e:
            self.gui.log_message(f"Error en autenticación: {str(e)}")
            return False
    
    def send_command(self, command):
        """Enviar comando al servidor"""
        if not self.connected or not self.socket:
            return False
        
        try:
            # Formatear mensaje según protocolo
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            message = f"{command}\r\nUSER: {self.username}\r\nTIMESTAMP: {timestamp}\r\n\r\n"
            
            self.socket.send(message.encode('utf-8'))
            self.gui.log_message(f"Enviado: {command}")
            return True
        except Exception as e:
            self.gui.log_message(f"Error enviando comando: {str(e)}")
            return False
    
    def send_vehicle_command(self, cmd):
        """Enviar comando de control del vehículo"""
        if not self.is_admin:
            self.gui.log_message("Error: Solo administradores pueden enviar comandos")
            return False
        
        valid_commands = ["SPEED_UP", "SLOW_DOWN", "TURN_LEFT", "TURN_RIGHT"]
        if cmd not in valid_commands:
            self.gui.log_message(f"Error: Comando no válido: {cmd}")
            return False
        
        return self.send_command(f"SEND_CMD: {cmd}")
    
    def request_data(self):
        """Solicitar datos de telemetría"""
        return self.send_command("GET_DATA:")
    
    def request_users_list(self):
        """Solicitar lista de usuarios conectados"""
        if not self.is_admin:
            self.gui.log_message("Error: Solo administradores pueden ver usuarios")
            return False
        return self.send_command("LIST_USERS:")
    
    def receive_messages(self):
        """Hilo para recibir mensajes del servidor"""
        while self.connected and self.running:
            try:
                data = self.socket.recv(1024)
                if not data:
                    self.gui.log_message("Servidor cerró la conexión")
                    break
                
                message = data.decode('utf-8').strip()
                self.process_server_message(message)
                
            except socket.timeout:
                continue
            except Exception as e:
                if self.connected:
                    self.gui.log_message(f"Error recibiendo mensaje: {str(e)}")
                break
    
    def process_server_message(self, message):
        """Procesar mensaje recibido del servidor"""
        try:
            # Log del mensaje recibido
            self.gui.log_message(f"Recibido: {message}")
            
            if message.startswith("AUTH_SUCCESS"):
                self.authenticated = True
                self.is_admin = True
                self.gui.on_authentication_success()
                
            elif message.startswith("AUTH_FAILED"):
                self.authenticated = False
                self.is_admin = False
                self.gui.on_authentication_failed()
                
            elif message.startswith("DATA:"):
                # Procesar datos de telemetría
                parts = message.split()
                if len(parts) >= 4:
                    self.vehicle_data['speed'] = int(parts[1])
                    self.vehicle_data['battery'] = int(parts[2])
                    self.vehicle_data['temperature'] = int(parts[3])
                    self.vehicle_data['direction'] = parts[4] if len(parts) > 4 else 'STRAIGHT'
                    self.gui.update_vehicle_data(self.vehicle_data)
                    
            elif message.startswith("OK:"):
                # Comando ejecutado exitosamente
                self.gui.log_message(f"Comando exitoso: {message[3:]}")
                
            elif message.startswith("ERROR:"):
                # Error en comando
                self.gui.log_message(f"Error: {message[6:]}")
                
            elif message.startswith("USERS:"):
                # Lista de usuarios conectados
                users_text = message[6:].strip()
                self.connected_users = users_text.split() if users_text else []
                self.gui.update_users_list(self.connected_users)
                
            else:
                self.gui.log_message(f"Mensaje no reconocido: {message}")
                
        except Exception as e:
            self.gui.log_message(f"Error procesando mensaje: {str(e)}")
    
    def run(self):
        """Ejecutar la aplicación GUI"""
        try:
            self.gui.run()
        except KeyboardInterrupt:
            self.gui.log_message("Interrumpido por usuario")
        finally:
            self.running = False
            self.disconnect()

def main():
    """Función principal"""
    print("Cliente de Telemetría Vehículo Autónomo")
    print("=====================================")
    
    # Obtener configuración del usuario
    host = input("Host del servidor [localhost]: ").strip() or "localhost"
    port_input = input("Puerto [8080]: ").strip()
    port = int(port_input) if port_input else 8080
    
    # Crear y ejecutar cliente
    client = TelemetryClient(host, port)
    
    try:
        # Conectar al servidor
        if client.connect():
            client.run()
        else:
            print("No se pudo conectar al servidor")
    except Exception as e:
        print(f"Error: {str(e)}")

if __name__ == "__main__":
    main()
