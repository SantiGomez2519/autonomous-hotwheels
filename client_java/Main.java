/**
 * Client graphical interface
 * Handles presentation and user events
 */
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

public class Main extends JFrame implements ActionListener, NetworkManager.NetworkEventListener {
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
    
    // Modelo de datos
    private VehicleData vehicleData;
    private List<String> connectedUsers;
    
    // Gestor de red
    private NetworkManager networkManager;
    
    public Main() {
        vehicleData = new VehicleData();
        connectedUsers = new ArrayList<>();
        networkManager = new NetworkManager(this);
        
        initializeGUI();
        setupEventHandlers();
        updateConnectionStatus();
    }
    
    private void initializeGUI() {
        setTitle("Autonomous Vehicle Telemetry Client - Refactored");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(900, 700);
        setLocationRelativeTo(null);
        
        // Main panel
        JPanel mainPanel = new JPanel(new BorderLayout());
        mainPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        
        // Top panel - Connection and authentication
        JPanel topPanel = createTopPanel();
        mainPanel.add(topPanel, BorderLayout.NORTH);
        
        // Center panel - Vehicle data and controls
        JPanel centerPanel = createCenterPanel();
        mainPanel.add(centerPanel, BorderLayout.CENTER);
        
        // Bottom panel - Log and users
        JPanel bottomPanel = createBottomPanel();
        mainPanel.add(bottomPanel, BorderLayout.SOUTH);
        
        add(mainPanel);
    }
    
    private JPanel createTopPanel() {
        JPanel panel = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        panel.setBorder(BorderFactory.createTitledBorder("Connection and Authentication"));
        
        // Connection configuration
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
        connectButton = new JButton("Connect");
        connectButton.addActionListener(this);
        panel.add(connectButton, gbc);
        
        gbc.gridx = 5;
        disconnectButton = new JButton("Disconnect");
        disconnectButton.addActionListener(this);
        disconnectButton.setEnabled(false);
        panel.add(disconnectButton, gbc);
        
        // Connection status
        gbc.gridx = 0; gbc.gridy = 1;
        panel.add(new JLabel("Estado:"), gbc);
        gbc.gridx = 1;
        statusLabel = new JLabel("Disconnected");
        panel.add(statusLabel, gbc);
        
        // Authentication
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
        authButton = new JButton("Authenticate");
        authButton.addActionListener(this);
        panel.add(authButton, gbc);
        
        gbc.gridx = 5;
        authLabel = new JLabel("Not authenticated");
        panel.add(authLabel, gbc);
        
        return panel;
    }
    
    private JPanel createCenterPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        
        // Vehicle data panel
        JPanel dataPanel = new JPanel(new GridLayout(2, 4, 10, 10));
        dataPanel.setBorder(BorderFactory.createTitledBorder("Vehicle Data"));
        
        dataPanel.add(new JLabel("Speed:"));
        speedLabel = new JLabel(vehicleData.getSpeedDisplay());
        speedLabel.setFont(new Font(Font.SANS_SERIF, Font.BOLD, 14));
        dataPanel.add(speedLabel);
        
        dataPanel.add(new JLabel("Battery:"));
        batteryLabel = new JLabel(vehicleData.getBatteryDisplay());
        batteryLabel.setFont(new Font(Font.SANS_SERIF, Font.BOLD, 14));
        dataPanel.add(batteryLabel);
        
        dataPanel.add(new JLabel("Temperature:"));
        temperatureLabel = new JLabel(vehicleData.getTemperatureDisplay());
        temperatureLabel.setFont(new Font(Font.SANS_SERIF, Font.BOLD, 14));
        dataPanel.add(temperatureLabel);
        
        dataPanel.add(new JLabel("Direction:"));
        directionLabel = new JLabel(vehicleData.getDirection());
        directionLabel.setFont(new Font(Font.SANS_SERIF, Font.BOLD, 14));
        dataPanel.add(directionLabel);
        
        panel.add(dataPanel, BorderLayout.NORTH);
        
        // Control panel
        JPanel controlPanel = new JPanel(new GridLayout(2, 3, 10, 10));
        controlPanel.setBorder(BorderFactory.createTitledBorder("Controls"));
        
        getDataButton = new JButton("Request Data");
        getDataButton.addActionListener(this);
        controlPanel.add(getDataButton);
        
        speedUpButton = new JButton("Speed Up");
        speedUpButton.addActionListener(this);
        speedUpButton.setEnabled(false);
        controlPanel.add(speedUpButton);
        
        slowDownButton = new JButton("Slow Down");
        slowDownButton.addActionListener(this);
        slowDownButton.setEnabled(false);
        controlPanel.add(slowDownButton);
        
        turnLeftButton = new JButton("Turn Left");
        turnLeftButton.addActionListener(this);
        turnLeftButton.setEnabled(false);
        controlPanel.add(turnLeftButton);
        
        turnRightButton = new JButton("Turn Right");
        turnRightButton.addActionListener(this);
        turnRightButton.setEnabled(false);
        controlPanel.add(turnRightButton);
        
        listUsersButton = new JButton("List Users");
        listUsersButton.addActionListener(this);
        listUsersButton.setEnabled(false);
        controlPanel.add(listUsersButton);
        
        panel.add(controlPanel, BorderLayout.CENTER);
        
        return panel;
    }
    
    private JPanel createBottomPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        
        // Connected users panel
        JPanel usersPanel = new JPanel(new BorderLayout());
        usersPanel.setBorder(BorderFactory.createTitledBorder("Connected Users"));
        usersPanel.setPreferredSize(new Dimension(200, 150));
        
        usersList = new JList<>();
        usersList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        JScrollPane usersScroll = new JScrollPane(usersList);
        usersPanel.add(usersScroll, BorderLayout.CENTER);
        
        panel.add(usersPanel, BorderLayout.EAST);
        
        // Log panel
        JPanel logPanel = new JPanel(new BorderLayout());
        logPanel.setBorder(BorderFactory.createTitledBorder("Message Log"));
        
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
        // Close window
        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                networkManager.disconnect();
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
            networkManager.disconnect();
        } else if (source == authButton) {
            authenticate();
        } else if (source == getDataButton) {
            networkManager.requestData();
        } else if (source == speedUpButton) {
            networkManager.sendVehicleCommand("SPEED_UP");
        } else if (source == slowDownButton) {
            networkManager.sendVehicleCommand("SLOW_DOWN");
        } else if (source == turnLeftButton) {
            networkManager.sendVehicleCommand("TURN_LEFT");
        } else if (source == turnRightButton) {
            networkManager.sendVehicleCommand("TURN_RIGHT");
        } else if (source == listUsersButton) {
            networkManager.requestUsersList();
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
            networkManager.connect(host, port);
        } catch (NumberFormatException e) {
            showMessage("Error", "Puerto inválido", JOptionPane.ERROR_MESSAGE);
        }
    }
    
    private void authenticate() {
        String username = usernameField.getText().trim();
        String password = new String(passwordField.getPassword()).trim();
        
        if (username.isEmpty() || password.isEmpty()) {
            showMessage("Error", "Usuario y contraseña son requeridos", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        networkManager.authenticate(username, password);
    }
    
    // Implementación de NetworkEventListener
    @Override
    public void onConnected() {
        SwingUtilities.invokeLater(() -> {
            updateConnectionStatus();
            logMessage("Conectado al servidor");
        });
    }
    
    @Override
    public void onDisconnected() {
        SwingUtilities.invokeLater(() -> {
            updateConnectionStatus();
            logMessage("Desconectado del servidor");
        });
    }
    
    @Override
    public void onAuthenticationSuccess() {
        SwingUtilities.invokeLater(() -> {
            authLabel.setText("Autenticado (Admin)");
            updateAdminControls(true);
            showMessage("Éxito", "Autenticación exitosa como administrador", JOptionPane.INFORMATION_MESSAGE);
        });
    }
    
    @Override
    public void onAuthenticationFailed() {
        SwingUtilities.invokeLater(() -> {
            authLabel.setText("Autenticación fallida");
            updateAdminControls(false);
            showMessage("Error", "Credenciales inválidas", JOptionPane.ERROR_MESSAGE);
        });
    }
    
    @Override
    public void onDataReceived(String message) {
        SwingUtilities.invokeLater(() -> {
            logMessage("Recibido: " + message);
            processServerMessage(message);
        });
    }
    
    @Override
    public void onError(String error) {
        SwingUtilities.invokeLater(() -> {
            logMessage("Error: " + error);
            showMessage("Error", error, JOptionPane.ERROR_MESSAGE);
        });
    }
    
    private void processServerMessage(String message) {
        if (message.startsWith("DATA:")) {
            // Procesar datos de telemetría
            String[] parts = message.split("\\s+");
            vehicleData.updateFromServerData(parts);
            updateVehicleDisplay();
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
        }
    }
    
    private void updateConnectionStatus() {
        if (networkManager.isConnected()) {
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
        speedLabel.setText(vehicleData.getSpeedDisplay());
        batteryLabel.setText(vehicleData.getBatteryDisplay());
        temperatureLabel.setText(vehicleData.getTemperatureDisplay());
        directionLabel.setText(vehicleData.getDirection());
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
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            // Usar look and feel por defecto si hay error
        }
        
        SwingUtilities.invokeLater(() -> {
            new Main().setVisible(true);
        });
    }
}
