/**
 * Cliente Java para Sistema de Telemetría Vehículo Autónomo
 * Implementa conexión TCP con el servidor y manejo de comandos
 * Soporta dos tipos de usuarios: Administrador y Observador
 * 
 * Compilación: javac Client.java
 * Ejecución: java Client
 */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

public class Client extends JFrame implements ActionListener {
    // Componentes de la interfaz
    private JLabel statusLabel, authLabel;
    private JTextField hostField, portField, usernameField;
    private JPasswordField passwordField;
    private JButton connectButton, disconnectButton, authButton;
    private JButton speedUpButton, slowDownButton, turnLeftButton, turnRightButton;
    private JButton getDataButton, listUsersButton;
    private JLabel speedLabel, batteryLabel, temperatureLabel, directionLabel;
    private JTextArea logArea;
    private JList<String> usersList;
    
    // Estado de conexión y datos
    private Socket socket;
    private BufferedReader in;
    private PrintWriter out;
    private AtomicBoolean connected = new AtomicBoolean(false);
    private AtomicBoolean authenticated = new AtomicBoolean(false);
    private AtomicBoolean isAdmin = new AtomicBoolean(false);
    private String username = "";
    
    // Datos del vehículo
    private int speed = 0;
    private int battery = 100;
    private int temperature = 20;
    private String direction = "STRAIGHT";
    
    // Lista de usuarios conectados
    private List<String> connectedUsers = new ArrayList<>();
    
    // Hilo para recibir mensajes
    private Thread receiveThread;
    
    public Client() {
        initializeGUI();
        setupEventHandlers();
    }
    
    private void initializeGUI() {
        setTitle("Cliente de Telemetría Vehículo Autónomo");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(900, 700);
        setLocationRelativeTo(null);
        
        // Panel principal
        JPanel mainPanel = new JPanel(new BorderLayout());
        mainPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        
        // Panel superior - Conexión y autenticación
        JPanel topPanel = createTopPanel();
        mainPanel.add(topPanel, BorderLayout.NORTH);
        
        // Panel central - Datos del vehículo y controles
        JPanel centerPanel = createCenterPanel();
        mainPanel.add(centerPanel, BorderLayout.CENTER);
        
        // Panel inferior - Log y usuarios
        JPanel bottomPanel = createBottomPanel();
        mainPanel.add(bottomPanel, BorderLayout.SOUTH);
        
        add(mainPanel);
        
        // Estado inicial
        updateConnectionStatus();
    }
    
    private JPanel createTopPanel() {
        JPanel panel = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        panel.setBorder(BorderFactory.createTitledBorder("Conexión y Autenticación"));
        
        // Configuración de conexión
        gbc.insets = new Insets(5, 5, 5, 5);
        gbc.anchor = GridBagConstraints.WEST;
        
        gbc.gridx = 0; gbc.gridy = 0;
        panel.add(new JLabel("Host:"), gbc);
        gbc.gridx = 1;
        hostField = new JTextField("localhost", 10);
        panel.add(hostField, gbc);
        
        gbc.gridx = 2;
        panel.add(new JLabel("Puerto:"), gbc);
        gbc.gridx = 3;
        portField = new JTextField("8080", 5);
        panel.add(portField, gbc);
        
        gbc.gridx = 4;
        connectButton = new JButton("Conectar");
        connectButton.addActionListener(this);
        panel.add(connectButton, gbc);
        
        gbc.gridx = 5;
        disconnectButton = new JButton("Desconectar");
        disconnectButton.addActionListener(this);
        disconnectButton.setEnabled(false);
        panel.add(disconnectButton, gbc);
        
        // Estado de conexión
        gbc.gridx = 0; gbc.gridy = 1;
        panel.add(new JLabel("Estado:"), gbc);
        gbc.gridx = 1;
        statusLabel = new JLabel("Desconectado");
        panel.add(statusLabel, gbc);
        
        // Autenticación
        gbc.gridx = 0; gbc.gridy = 2;
        panel.add(new JLabel("Usuario:"), gbc);
        gbc.gridx = 1;
        usernameField = new JTextField("admin", 10);
        panel.add(usernameField, gbc);
        
        gbc.gridx = 2;
        panel.add(new JLabel("Contraseña:"), gbc);
        gbc.gridx = 3;
        passwordField = new JPasswordField("admin123", 10);
        panel.add(passwordField, gbc);
        
        gbc.gridx = 4;
        authButton = new JButton("Autenticar");
        authButton.addActionListener(this);
        panel.add(authButton, gbc);
        
        gbc.gridx = 5;
        authLabel = new JLabel("No autenticado");
        panel.add(authLabel, gbc);
        
        return panel;
    }
    
    private JPanel createCenterPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        
        // Panel de datos del vehículo
        JPanel dataPanel = new JPanel(new GridLayout(2, 4, 10, 10));
        dataPanel.setBorder(BorderFactory.createTitledBorder("Datos del Vehículo"));
        
        dataPanel.add(new JLabel("Velocidad:"));
        speedLabel = new JLabel("0 km/h");
        speedLabel.setFont(new Font(Font.SANS_SERIF, Font.BOLD, 14));
        dataPanel.add(speedLabel);
        
        dataPanel.add(new JLabel("Batería:"));
        batteryLabel = new JLabel("100%");
        batteryLabel.setFont(new Font(Font.SANS_SERIF, Font.BOLD, 14));
        dataPanel.add(batteryLabel);
        
        dataPanel.add(new JLabel("Temperatura:"));
        temperatureLabel = new JLabel("20°C");
        temperatureLabel.setFont(new Font(Font.SANS_SERIF, Font.BOLD, 14));
        dataPanel.add(temperatureLabel);
        
        dataPanel.add(new JLabel("Dirección:"));
        directionLabel = new JLabel("STRAIGHT");
        directionLabel.setFont(new Font(Font.SANS_SERIF, Font.BOLD, 14));
        dataPanel.add(directionLabel);
        
        panel.add(dataPanel, BorderLayout.NORTH);
        
        // Panel de controles
        JPanel controlPanel = new JPanel(new GridLayout(2, 3, 10, 10));
        controlPanel.setBorder(BorderFactory.createTitledBorder("Controles"));
        
        getDataButton = new JButton("Solicitar Datos");
        getDataButton.addActionListener(this);
        controlPanel.add(getDataButton);
        
        speedUpButton = new JButton("Acelerar");
        speedUpButton.addActionListener(this);
        speedUpButton.setEnabled(false);
        controlPanel.add(speedUpButton);
        
        slowDownButton = new JButton("Frenar");
        slowDownButton.addActionListener(this);
        slowDownButton.setEnabled(false);
        controlPanel.add(slowDownButton);
        
        turnLeftButton = new JButton("Izquierda");
        turnLeftButton.addActionListener(this);
        turnLeftButton.setEnabled(false);
        controlPanel.add(turnLeftButton);
        
        turnRightButton = new JButton("Derecha");
        turnRightButton.addActionListener(this);
        turnRightButton.setEnabled(false);
        controlPanel.add(turnRightButton);
        
        listUsersButton = new JButton("Listar Usuarios");
        listUsersButton.addActionListener(this);
        listUsersButton.setEnabled(false);
        controlPanel.add(listUsersButton);
        
        panel.add(controlPanel, BorderLayout.CENTER);
        
        return panel;
    }
    
    private JPanel createBottomPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        
        // Panel de usuarios conectados
        JPanel usersPanel = new JPanel(new BorderLayout());
        usersPanel.setBorder(BorderFactory.createTitledBorder("Usuarios Conectados"));
        usersPanel.setPreferredSize(new Dimension(200, 150));
        
        usersList = new JList<>();
        usersList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        JScrollPane usersScroll = new JScrollPane(usersList);
        usersPanel.add(usersScroll, BorderLayout.CENTER);
        
        panel.add(usersPanel, BorderLayout.EAST);
        
        // Panel de log
        JPanel logPanel = new JPanel(new BorderLayout());
        logPanel.setBorder(BorderFactory.createTitledBorder("Log de Mensajes"));
        
        logArea = new JTextArea(10, 50);
        logArea.setEditable(false);
        logArea.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));
        JScrollPane logScroll = new JScrollPane(logArea);
        logScroll.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        logPanel.add(logScroll, BorderLayout.CENTER);
        
        panel.add(logPanel, BorderLayout.CENTER);
        
        return panel;
    }
    
    private void setupEventHandlers() {
        // Cerrar ventana
        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                disconnect();
                System.exit(0);
            }
        });
    }
    
    @Override
    public void actionPerformed(ActionEvent e) {
        Object source = e.getSource();
        
        if (source == connectButton) {
            connect();
        } else if (source == disconnectButton) {
            disconnect();
        } else if (source == authButton) {
            authenticate();
        } else if (source == getDataButton) {
            requestData();
        } else if (source == speedUpButton) {
            sendVehicleCommand("SPEED_UP");
        } else if (source == slowDownButton) {
            sendVehicleCommand("SLOW_DOWN");
        } else if (source == turnLeftButton) {
            sendVehicleCommand("TURN_LEFT");
        } else if (source == turnRightButton) {
            sendVehicleCommand("TURN_RIGHT");
        } else if (source == listUsersButton) {
            requestUsersList();
        }
    }
    
    private void connect() {
        String host = hostField.getText().trim();
        String portText = portField.getText().trim();
        
        if (host.isEmpty() || portText.isEmpty()) {
            showMessage("Error", "Host y puerto son requeridos", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        try {
            int port = Integer.parseInt(portText);
            socket = new Socket(host, port);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);
            
            connected.set(true);
            updateConnectionStatus();
            logMessage("Conectado al servidor " + host + ":" + port);
            
            // Iniciar hilo para recibir mensajes
            receiveThread = new Thread(this::receiveMessages);
            receiveThread.setDaemon(true);
            receiveThread.start();
            
        } catch (NumberFormatException e) {
            showMessage("Error", "Puerto inválido", JOptionPane.ERROR_MESSAGE);
        } catch (IOException e) {
            showMessage("Error", "No se pudo conectar al servidor: " + e.getMessage(), JOptionPane.ERROR_MESSAGE);
            logMessage("Error conectando: " + e.getMessage());
        }
    }
    
    private void disconnect() {
        if (connected.get()) {
            try {
                sendCommand("DISCONNECT:");
                connected.set(false);
                authenticated.set(false);
                isAdmin.set(false);
                username = "";
                
                if (socket != null) {
                    socket.close();
                }
                
                updateConnectionStatus();
                logMessage("Desconectado del servidor");
                
            } catch (IOException e) {
                logMessage("Error desconectando: " + e.getMessage());
            }
        }
    }
    
    private void authenticate() {
        if (!connected.get()) {
            showMessage("Error", "No hay conexión con el servidor", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        String username = usernameField.getText().trim();
        String password = new String(passwordField.getPassword()).trim();
        
        if (username.isEmpty() || password.isEmpty()) {
            showMessage("Error", "Usuario y contraseña son requeridos", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        this.username = username;
        sendCommand("AUTH: " + username + " " + password);
        logMessage("Intentando autenticación como: " + username);
    }
    
    private void requestData() {
        if (!connected.get()) {
            showMessage("Error", "No hay conexión con el servidor", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        sendCommand("GET_DATA:");
        logMessage("Solicitando datos de telemetría");
    }
    
    private void sendVehicleCommand(String command) {
        if (!connected.get()) {
            showMessage("Error", "No hay conexión con el servidor", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        if (!isAdmin.get()) {
            showMessage("Error", "Solo administradores pueden enviar comandos", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        sendCommand("SEND_CMD: " + command);
        logMessage("Enviando comando: " + command);
    }
    
    private void requestUsersList() {
        if (!connected.get()) {
            showMessage("Error", "No hay conexión con el servidor", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        if (!isAdmin.get()) {
            showMessage("Error", "Solo administradores pueden ver usuarios", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        sendCommand("LIST_USERS:");
        logMessage("Solicitando lista de usuarios");
    }
    
    private void sendCommand(String command) {
        if (out != null) {
            String timestamp = LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"));
            String message = command + "\r\nUSER: " + username + "\r\nTIMESTAMP: " + timestamp + "\r\n\r\n";
            out.println(message);
            logMessage("Enviado: " + command);
        }
    }
    
    private void receiveMessages() {
        try {
            String line;
            StringBuilder message = new StringBuilder();
            
            while (connected.get() && (line = in.readLine()) != null) {
                if (line.trim().isEmpty()) {
                    // Fin del mensaje
                    processServerMessage(message.toString());
                    message = new StringBuilder();
                } else {
                    message.append(line).append("\n");
                }
            }
        } catch (IOException e) {
            if (connected.get()) {
                SwingUtilities.invokeLater(() -> {
                    logMessage("Error recibiendo mensajes: " + e.getMessage());
                    disconnect();
                });
            }
        }
    }
    
    private void processServerMessage(String message) {
        SwingUtilities.invokeLater(() -> {
            logMessage("Recibido: " + message);
            
            if (message.startsWith("AUTH_SUCCESS")) {
                authenticated.set(true);
                isAdmin.set(true);
                authLabel.setText("Autenticado (Admin)");
                updateAdminControls(true);
                showMessage("Éxito", "Autenticación exitosa como administrador", JOptionPane.INFORMATION_MESSAGE);
                
            } else if (message.startsWith("AUTH_FAILED")) {
                authenticated.set(false);
                isAdmin.set(false);
                authLabel.setText("Autenticación fallida");
                updateAdminControls(false);
                showMessage("Error", "Credenciales inválidas", JOptionPane.ERROR_MESSAGE);
                
            } else if (message.startsWith("DATA:")) {
                // Procesar datos de telemetría
                String[] parts = message.split("\\s+");
                if (parts.length >= 4) {
                    try {
                        speed = Integer.parseInt(parts[1]);
                        battery = Integer.parseInt(parts[2]);
                        temperature = Integer.parseInt(parts[3]);
                        direction = parts.length > 4 ? parts[4] : "STRAIGHT";
                        
                        updateVehicleDisplay();
                        
                    } catch (NumberFormatException e) {
                        logMessage("Error parseando datos de telemetría");
                    }
                }
                
            } else if (message.startsWith("OK:")) {
                String response = message.substring(3).trim();
                logMessage("Comando exitoso: " + response);
                
            } else if (message.startsWith("ERROR:")) {
                String error = message.substring(6).trim();
                logMessage("Error: " + error);
                showMessage("Error", error, JOptionPane.ERROR_MESSAGE);
                
            } else if (message.startsWith("USERS:")) {
                String usersText = message.substring(6).trim();
                connectedUsers.clear();
                if (!usersText.isEmpty()) {
                    String[] users = usersText.split("\\s+");
                    for (String user : users) {
                        connectedUsers.add(user);
                    }
                }
                updateUsersList();
                
            } else {
                logMessage("Mensaje no reconocido: " + message);
            }
        });
    }
    
    private void updateConnectionStatus() {
        if (connected.get()) {
            statusLabel.setText("Conectado");
            connectButton.setEnabled(false);
            disconnectButton.setEnabled(true);
            authButton.setEnabled(true);
            getDataButton.setEnabled(true);
        } else {
            statusLabel.setText("Desconectado");
            connectButton.setEnabled(true);
            disconnectButton.setEnabled(false);
            authButton.setEnabled(false);
            getDataButton.setEnabled(false);
            updateAdminControls(false);
            authLabel.setText("No autenticado");
        }
    }
    
    private void updateAdminControls(boolean enabled) {
        speedUpButton.setEnabled(enabled);
        slowDownButton.setEnabled(enabled);
        turnLeftButton.setEnabled(enabled);
        turnRightButton.setEnabled(enabled);
        listUsersButton.setEnabled(enabled);
    }
    
    private void updateVehicleDisplay() {
        speedLabel.setText(speed + " km/h");
        batteryLabel.setText(battery + "%");
        temperatureLabel.setText(temperature + "°C");
        directionLabel.setText(direction);
    }
    
    private void updateUsersList() {
        DefaultListModel<String> model = new DefaultListModel<>();
        for (String user : connectedUsers) {
            model.addElement(user);
        }
        usersList.setModel(model);
    }
    
    private void logMessage(String message) {
        String timestamp = LocalDateTime.now().format(DateTimeFormatter.ofPattern("HH:mm:ss"));
        String logEntry = "[" + timestamp + "] " + message + "\n";
        
        SwingUtilities.invokeLater(() -> {
            logArea.append(logEntry);
            logArea.setCaretPosition(logArea.getDocument().getLength());
        });
    }
    
    private void showMessage(String title, String message, int messageType) {
        SwingUtilities.invokeLater(() -> {
            JOptionPane.showMessageDialog(this, message, title, messageType);
        });
    }
    
    public static void main(String[] args) {
        // Configurar look and feel del sistema
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeel());
        } catch (Exception e) {
            // Usar look and feel por defecto si hay error
        }
        
        SwingUtilities.invokeLater(() -> {
            new Client().setVisible(true);
        });
    }
}
