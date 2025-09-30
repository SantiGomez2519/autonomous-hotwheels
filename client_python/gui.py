#!/usr/bin/env python3
"""
GUI para Cliente de Telemetría Vehículo Autónomo
Interfaz gráfica usando tkinter para mostrar datos y controles
"""

import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import threading
from datetime import datetime

class TelemetryGUI:
    def __init__(self, client):
        self.client = client
        
        # Crear ventana principal
        self.root = tk.Tk()
        self.root.title("Cliente de Telemetría Vehículo Autónomo")
        self.root.geometry("800x600")
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
        # Variables de la interfaz
        self.connected_var = tk.StringVar(value="Desconectado")
        self.auth_var = tk.StringVar(value="No autenticado")
        self.speed_var = tk.StringVar(value="0 km/h")
        self.battery_var = tk.StringVar(value="100%")
        self.temperature_var = tk.StringVar(value="20°C")
        self.direction_var = tk.StringVar(value="STRAIGHT")
        
        # Crear interfaz
        self.create_widgets()
        
        # Actualizar estado inicial
        self.update_connection_status()
    
    def create_widgets(self):
        """Crear todos los widgets de la interfaz"""
        
        # Frame principal
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Configurar grid
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        
        # === SECCIÓN DE ESTADO ===
        status_frame = ttk.LabelFrame(main_frame, text="Estado de Conexión", padding="5")
        status_frame.grid(row=0, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        ttk.Label(status_frame, text="Conexión:").grid(row=0, column=0, sticky=tk.W)
        ttk.Label(status_frame, textvariable=self.connected_var).grid(row=0, column=1, sticky=tk.W, padx=(10, 0))
        
        ttk.Label(status_frame, text="Autenticación:").grid(row=1, column=0, sticky=tk.W)
        ttk.Label(status_frame, textvariable=self.auth_var).grid(row=1, column=1, sticky=tk.W, padx=(10, 0))
        
        # === SECCIÓN DE AUTENTICACIÓN ===
        auth_frame = ttk.LabelFrame(main_frame, text="Autenticación", padding="5")
        auth_frame.grid(row=1, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        ttk.Label(auth_frame, text="Usuario:").grid(row=0, column=0, sticky=tk.W)
        self.username_entry = ttk.Entry(auth_frame, width=20)
        self.username_entry.grid(row=0, column=1, sticky=tk.W, padx=(5, 10))
        self.username_entry.insert(0, "admin")
        
        ttk.Label(auth_frame, text="Contraseña:").grid(row=0, column=2, sticky=tk.W)
        self.password_entry = ttk.Entry(auth_frame, width=20, show="*")
        self.password_entry.grid(row=0, column=3, sticky=tk.W, padx=(5, 10))
        self.password_entry.insert(0, "admin123")
        
        self.auth_button = ttk.Button(auth_frame, text="Autenticar", command=self.authenticate)
        self.auth_button.grid(row=0, column=4, padx=(10, 0))
        
        # === SECCIÓN DE DATOS DEL VEHÍCULO ===
        vehicle_frame = ttk.LabelFrame(main_frame, text="Datos del Vehículo", padding="5")
        vehicle_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        # Velocidad
        ttk.Label(vehicle_frame, text="Velocidad:").grid(row=0, column=0, sticky=tk.W)
        ttk.Label(vehicle_frame, textvariable=self.speed_var, font=("Arial", 12, "bold")).grid(row=0, column=1, sticky=tk.W, padx=(10, 20))
        
        # Batería
        ttk.Label(vehicle_frame, text="Batería:").grid(row=0, column=2, sticky=tk.W)
        ttk.Label(vehicle_frame, textvariable=self.battery_var, font=("Arial", 12, "bold")).grid(row=0, column=3, sticky=tk.W, padx=(10, 20))
        
        # Temperatura
        ttk.Label(vehicle_frame, text="Temperatura:").grid(row=1, column=0, sticky=tk.W)
        ttk.Label(vehicle_frame, textvariable=self.temperature_var, font=("Arial", 12, "bold")).grid(row=1, column=1, sticky=tk.W, padx=(10, 20))
        
        # Dirección
        ttk.Label(vehicle_frame, text="Dirección:").grid(row=1, column=2, sticky=tk.W)
        ttk.Label(vehicle_frame, textvariable=self.direction_var, font=("Arial", 12, "bold")).grid(row=1, column=3, sticky=tk.W, padx=(10, 20))
        
        # Botón para solicitar datos
        ttk.Button(vehicle_frame, text="Solicitar Datos", command=self.request_data).grid(row=2, column=0, pady=(10, 0))
        
        # === SECCIÓN DE CONTROLES (solo para administradores) ===
        self.control_frame = ttk.LabelFrame(main_frame, text="Controles del Vehículo", padding="5")
        self.control_frame.grid(row=3, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        # Botones de control
        control_buttons = [
            ("Acelerar", "SPEED_UP"),
            ("Frenar", "SLOW_DOWN"),
            ("Izquierda", "TURN_LEFT"),
            ("Derecha", "TURN_RIGHT")
        ]
        
        for i, (text, command) in enumerate(control_buttons):
            btn = ttk.Button(self.control_frame, text=text, 
                           command=lambda cmd=command: self.send_vehicle_command(cmd))
            btn.grid(row=i//2, column=i%2, padx=5, pady=2, sticky=(tk.W, tk.E))
        
        # Botón para listar usuarios
        self.users_button = ttk.Button(self.control_frame, text="Listar Usuarios", 
                                     command=self.request_users_list)
        self.users_button.grid(row=2, column=0, columnspan=2, pady=(10, 0))
        
        # Inicialmente ocultar controles
        self.hide_admin_controls()
        
        # === SECCIÓN DE USUARIOS CONECTADOS ===
        self.users_frame = ttk.LabelFrame(main_frame, text="Usuarios Conectados", padding="5")
        self.users_frame.grid(row=4, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        self.users_listbox = tk.Listbox(self.users_frame, height=4)
        self.users_listbox.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        users_scrollbar = ttk.Scrollbar(self.users_frame, orient=tk.VERTICAL, command=self.users_listbox.yview)
        users_scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        self.users_listbox.configure(yscrollcommand=users_scrollbar.set)
        
        self.users_frame.columnconfigure(0, weight=1)
        
        # === SECCIÓN DE LOG ===
        log_frame = ttk.LabelFrame(main_frame, text="Log de Mensajes", padding="5")
        log_frame.grid(row=5, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=10, width=80)
        self.log_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(0, weight=1)
        main_frame.rowconfigure(5, weight=1)
        
        # === BOTONES DE CONEXIÓN ===
        button_frame = ttk.Frame(main_frame)
        button_frame.grid(row=6, column=0, columnspan=2, pady=(10, 0))
        
        self.connect_button = ttk.Button(button_frame, text="Conectar", command=self.connect)
        self.connect_button.pack(side=tk.LEFT, padx=(0, 10))
        
        self.disconnect_button = ttk.Button(button_frame, text="Desconectar", command=self.disconnect)
        self.disconnect_button.pack(side=tk.LEFT)
        
        # Estado inicial de botones
        self.disconnect_button.config(state=tk.DISABLED)
    
    def update_connection_status(self):
        """Actualizar estado de conexión en la interfaz"""
        if self.client.connected:
            self.connected_var.set("Conectado")
            self.connect_button.config(state=tk.DISABLED)
            self.disconnect_button.config(state=tk.NORMAL)
        else:
            self.connected_var.set("Desconectado")
            self.connect_button.config(state=tk.NORMAL)
            self.disconnect_button.config(state=tk.DISABLED)
            self.hide_admin_controls()
    
    def show_admin_controls(self):
        """Mostrar controles de administrador"""
        for widget in self.control_frame.winfo_children():
            if isinstance(widget, ttk.Button) and widget['text'] not in ['Acelerar', 'Frenar', 'Izquierda', 'Derecha']:
                continue
            widget.config(state=tk.NORMAL)
        self.users_button.config(state=tk.NORMAL)
    
    def hide_admin_controls(self):
        """Ocultar controles de administrador"""
        for widget in self.control_frame.winfo_children():
            if isinstance(widget, ttk.Button) and widget['text'] not in ['Acelerar', 'Frenar', 'Izquierda', 'Derecha']:
                continue
            widget.config(state=tk.DISABLED)
        self.users_button.config(state=tk.DISABLED)
    
    def authenticate(self):
        """Autenticar usuario"""
        username = self.username_entry.get().strip()
        password = self.password_entry.get().strip()
        
        if not username or not password:
            messagebox.showerror("Error", "Usuario y contraseña son requeridos")
            return
        
        if not self.client.connected:
            messagebox.showerror("Error", "No hay conexión con el servidor")
            return
        
        # Ejecutar autenticación en hilo separado para no bloquear GUI
        threading.Thread(target=self.client.authenticate, args=(username, password), daemon=True).start()
    
    def on_authentication_success(self):
        """Callback para autenticación exitosa"""
        self.auth_var.set("Autenticado (Admin)")
        self.client.username = self.username_entry.get().strip()
        self.show_admin_controls()
        messagebox.showinfo("Éxito", "Autenticación exitosa como administrador")
    
    def on_authentication_failed(self):
        """Callback para autenticación fallida"""
        self.auth_var.set("Autenticación fallida")
        self.client.username = ""
        self.hide_admin_controls()
        messagebox.showerror("Error", "Credenciales inválidas")
    
    def update_vehicle_data(self, data):
        """Actualizar datos del vehículo en la interfaz"""
        self.speed_var.set(f"{data['speed']} km/h")
        self.battery_var.set(f"{data['battery']}%")
        self.temperature_var.set(f"{data['temperature']}°C")
        self.direction_var.set(data['direction'])
    
    def request_data(self):
        """Solicitar datos de telemetría"""
        if not self.client.connected:
            messagebox.showerror("Error", "No hay conexión con el servidor")
            return
        
        threading.Thread(target=self.client.request_data, daemon=True).start()
    
    def send_vehicle_command(self, command):
        """Enviar comando de control del vehículo"""
        if not self.client.connected:
            messagebox.showerror("Error", "No hay conexión con el servidor")
            return
        
        threading.Thread(target=self.client.send_vehicle_command, args=(command,), daemon=True).start()
    
    def request_users_list(self):
        """Solicitar lista de usuarios conectados"""
        if not self.client.connected:
            messagebox.showerror("Error", "No hay conexión con el servidor")
            return
        
        threading.Thread(target=self.client.request_users_list, daemon=True).start()
    
    def update_users_list(self, users):
        """Actualizar lista de usuarios conectados"""
        self.users_listbox.delete(0, tk.END)
        for user in users:
            self.users_listbox.insert(tk.END, user)
    
    def log_message(self, message):
        """Agregar mensaje al log"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        log_entry = f"[{timestamp}] {message}\n"
        
        # Ejecutar en hilo principal para evitar problemas de concurrencia
        self.root.after(0, self._append_log, log_entry)
    
    def _append_log(self, message):
        """Agregar mensaje al log (ejecutar en hilo principal)"""
        self.log_text.insert(tk.END, message)
        self.log_text.see(tk.END)
    
    def connect(self):
        """Conectar al servidor"""
        threading.Thread(target=self._connect_thread, daemon=True).start()
    
    def _connect_thread(self):
        """Hilo para conectar al servidor"""
        success = self.client.connect()
        self.root.after(0, self._on_connect_result, success)
    
    def _on_connect_result(self, success):
        """Callback para resultado de conexión"""
        self.update_connection_status()
        if success:
            messagebox.showinfo("Éxito", "Conectado al servidor")
        else:
            messagebox.showerror("Error", "No se pudo conectar al servidor")
    
    def disconnect(self):
        """Desconectar del servidor"""
        self.client.disconnect()
        self.update_connection_status()
        self.auth_var.set("No autenticado")
        self.client.username = ""
        self.hide_admin_controls()
    
    def on_closing(self):
        """Manejar cierre de ventana"""
        self.client.running = False
        self.client.disconnect()
        self.root.destroy()
    
    def run(self):
        """Ejecutar la aplicación GUI"""
        self.root.mainloop()
