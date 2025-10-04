#!/usr/bin/env python3
"""
Gestor de comunicación de red
Maneja la conexión TCP y el protocolo de comunicación
"""

import socket
import threading
from datetime import datetime
from typing import Callable, Optional

class NetworkManager:
    def __init__(self):
        self.host = 'localhost'
        self.port = 8080
        self.socket = None
        self.connected = False
        self.authenticated = False
        self.is_admin = False
        self.username = ""
        self.running = True
        
        # Callbacks para eventos
        self.on_connected: Optional[Callable] = None
        self.on_disconnected: Optional[Callable] = None
        self.on_authentication_success: Optional[Callable] = None
        self.on_authentication_failed: Optional[Callable] = None
        self.on_data_received: Optional[Callable[[str], None]] = None
        self.on_error: Optional[Callable[[str], None]] = None
        
        # Hilo de recepción
        self.receive_thread = None
    
    def connect(self, host: str = 'localhost', port: int = 8080) -> bool:
        """Conectar al servidor"""
        try:
            self.host = host
            self.port = port
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(10)  # Timeout de 10 segundos
            self.socket.connect((host, port))
            self.connected = True
            self.authenticated = False
            self.is_admin = False
            self.username = ""
            
            # Iniciar hilo para recibir mensajes
            self.receive_thread = threading.Thread(target=self._receive_messages)
            self.receive_thread.daemon = True
            self.receive_thread.start()
            
            if self.on_connected:
                self.on_connected()
            
            return True
        except Exception as e:
            if self.on_error:
                self.on_error(f"Error conectando: {str(e)}")
            return False
    
    def disconnect(self):
        """Desconectar del servidor"""
        try:
            if self.connected:
                self._send_command("DISCONNECT:")
                self.connected = False
                self.authenticated = False
                self.is_admin = False
                self.running = False
                self.socket.close()
                if self.on_disconnected:
                    self.on_disconnected()
        except Exception as e:
            if self.on_error:
                self.on_error(f"Error desconectando: {str(e)}")
    
    def authenticate(self, username: str, password: str) -> bool:
        """Autenticar como administrador"""
        if not self.connected:
            if self.on_error:
                self.on_error("No hay conexión con el servidor")
            return False
        
        try:
            self.username = username
            command = f"AUTH: {username} {password}"
            self._send_command(command)
            return True
        except Exception as e:
            if self.on_error:
                self.on_error(f"Error en autenticación: {str(e)}")
            return False
    
    def request_data(self) -> bool:
        """Solicitar datos de telemetría"""
        if not self.connected:
            if self.on_error:
                self.on_error("No hay conexión con el servidor")
            return False
        return self._send_command("GET_DATA:")
    
    def send_vehicle_command(self, cmd: str) -> bool:
        """Enviar comando de control del vehículo"""
        if not self.connected:
            if self.on_error:
                self.on_error("No hay conexión con el servidor")
            return False
        
        if not self.is_admin:
            if self.on_error:
                self.on_error("Solo administradores pueden enviar comandos")
            return False
        
        valid_commands = ["SPEED_UP", "SLOW_DOWN", "TURN_LEFT", "TURN_RIGHT"]
        if cmd not in valid_commands:
            if self.on_error:
                self.on_error(f"Comando no válido: {cmd}")
            return False
        
        return self._send_command(f"SEND_CMD: {cmd}")
    
    def request_users_list(self) -> bool:
        """Solicitar lista de usuarios conectados"""
        if not self.connected:
            if self.on_error:
                self.on_error("No hay conexión con el servidor")
            return False
        
        if not self.is_admin:
            if self.on_error:
                self.on_error("Solo administradores pueden ver usuarios")
            return False
        
        return self._send_command("LIST_USERS:")
    
    def _send_command(self, command: str) -> bool:
        """Enviar comando al servidor"""
        if not self.connected or not self.socket:
            return False
        
        try:
            # Formatear mensaje según protocolo
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            message = f"{command}\r\nUSER: {self.username}\r\nTIMESTAMP: {timestamp}\r\n\r\n"
            
            self.socket.send(message.encode('utf-8'))
            return True
        except Exception as e:
            if self.on_error:
                self.on_error(f"Error enviando comando: {str(e)}")
            return False
    
    def _receive_messages(self):
        """Hilo para recibir mensajes del servidor"""
        while self.connected and self.running:
            try:
                data = self.socket.recv(1024)
                if not data:
                    if self.on_error:
                        self.on_error("Servidor cerró la conexión")
                    break
                
                message = data.decode('utf-8').strip()
                self._process_server_message(message)
                
            except socket.timeout:
                continue
            except Exception as e:
                if self.connected and self.on_error:
                    self.on_error(f"Error recibiendo mensaje: {str(e)}")
                break
    
    def _process_server_message(self, message: str):
        """Procesar mensaje recibido del servidor"""
        try:
            if self.on_data_received:
                self.on_data_received(message)
            
            if message.startswith("AUTH_SUCCESS"):
                self.authenticated = True
                self.is_admin = True
                if self.on_authentication_success:
                    self.on_authentication_success()
                    
            elif message.startswith("AUTH_FAILED"):
                self.authenticated = False
                self.is_admin = False
                if self.on_authentication_failed:
                    self.on_authentication_failed()
                    
        except Exception as e:
            if self.on_error:
                self.on_error(f"Error procesando mensaje: {str(e)}")
    
    # Getters para estado
    def is_connected(self) -> bool:
        return self.connected
    
    def is_authenticated(self) -> bool:
        return self.authenticated
    
    def is_admin_user(self) -> bool:
        return self.is_admin
    
    def get_username(self) -> str:
        return self.username
