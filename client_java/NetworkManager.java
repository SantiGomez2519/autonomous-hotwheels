/**
 * Network communication manager
 * Handles TCP connection and communication protocol
 */
import java.io.*;
import java.net.*;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.concurrent.atomic.AtomicBoolean;

public class NetworkManager {
    private Socket socket;
    private BufferedReader in;
    private PrintWriter out;
    private AtomicBoolean connected = new AtomicBoolean(false);
    private AtomicBoolean authenticated = new AtomicBoolean(false);
    private AtomicBoolean isAdmin = new AtomicBoolean(false);
    private String username = "";
    private Thread receiveThread;
    
    // Callbacks for network events
    public interface NetworkEventListener {
        void onConnected();
        void onDisconnected();
        void onAuthenticationSuccess();
        void onAuthenticationFailed();
        void onDataReceived(String data);
        void onError(String error);
    }
    
    private NetworkEventListener listener;
    
    public NetworkManager(NetworkEventListener listener) {
        this.listener = listener;
    }
    
    public boolean connect(String host, int port) {
        try {
            socket = new Socket(host, port);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);
            
            connected.set(true);
            username = "";
            authenticated.set(false);
            isAdmin.set(false);
            
            // Start thread to receive messages
            receiveThread = new Thread(this::receiveMessages);
            receiveThread.setDaemon(true);
            receiveThread.start();
            
            if (listener != null) {
                listener.onConnected();
            }
            
            return true;
        } catch (IOException e) {
            if (listener != null) {
                listener.onError("Error connecting: " + e.getMessage());
            }
            return false;
        }
    }
    
    public void disconnect() {
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
                
                if (listener != null) {
                    listener.onDisconnected();
                }
            } catch (IOException e) {
                if (listener != null) {
                    listener.onError("Error desconectando: " + e.getMessage());
                }
            }
        }
    }
    
    public void authenticate(String username, String password) {
        if (!connected.get()) {
            if (listener != null) {
                listener.onError("No connection to server");
            }
            return;
        }
        
        this.username = username;
        sendCommand("AUTH: " + username + " " + password);
    }
    
    public void requestData() {
        if (!connected.get()) {
            if (listener != null) {
                listener.onError("No connection to server");
            }
            return;
        }
        sendCommand("GET_DATA:");
    }
    
    public void sendVehicleCommand(String command) {
        if (!connected.get()) {
            if (listener != null) {
                listener.onError("No connection to server");
            }
            return;
        }
        
        if (!isAdmin.get()) {
            if (listener != null) {
                listener.onError("Only administrators can send commands");
            }
            return;
        }
        
        sendCommand("SEND_CMD: " + command);
    }
    
    public void requestUsersList() {
        if (!connected.get()) {
            if (listener != null) {
                listener.onError("No connection to server");
            }
            return;
        }
        
        if (!isAdmin.get()) {
            if (listener != null) {
                listener.onError("Only administrators can view users");
            }
            return;
        }
        
        sendCommand("LIST_USERS:");
    }
    
    private void sendCommand(String command) {
        if (out != null) {
            String timestamp = LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"));
            String message = command + "\r\nUSER: " + username + "\r\nTIMESTAMP: " + timestamp + "\r\n\r\n";
            out.println(message);
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
            if (connected.get() && listener != null) {
                listener.onError("Error recibiendo mensajes: " + e.getMessage());
                disconnect();
            }
        }
    }
    
    private void processServerMessage(String message) {
        if (listener != null) {
            listener.onDataReceived(message);
        }
        
        if (message.startsWith("AUTH_SUCCESS")) {
            authenticated.set(true);
            isAdmin.set(true);
            if (listener != null) {
                listener.onAuthenticationSuccess();
            }
        } else if (message.startsWith("AUTH_FAILED")) {
            authenticated.set(false);
            isAdmin.set(false);
            if (listener != null) {
                listener.onAuthenticationFailed();
            }
        }
    }
    
    // Getters para estado
    public boolean isConnected() { return connected.get(); }
    public boolean isAuthenticated() { return authenticated.get(); }
    public boolean isAdmin() { return isAdmin.get(); }
    public String getUsername() { return username; }
}
