/**
 * Modelo de datos del vehículo
 * Contiene el estado actual del vehículo y métodos para actualizarlo
 */
public class VehicleData {
    private int speed = 0;
    private int battery = 100;
    private int temperature = 20;
    private String direction = "STRAIGHT";
    
    // Getters
    public int getSpeed() { return speed; }
    public int getBattery() { return battery; }
    public int getTemperature() { return temperature; }
    public String getDirection() { return direction; }
    
    // Setters
    public void setSpeed(int speed) { 
        this.speed = Math.max(0, Math.min(100, speed)); 
    }
    
    public void setBattery(int battery) { 
        this.battery = Math.max(0, Math.min(100, battery)); 
    }
    
    public void setTemperature(int temperature) { 
        this.temperature = temperature; 
    }
    
    public void setDirection(String direction) { 
        this.direction = direction != null ? direction : "STRAIGHT"; 
    }
    
    // Actualizar desde datos del servidor
    public void updateFromServerData(String[] parts) {
        if (parts.length >= 4) {
            try {
                setSpeed(Integer.parseInt(parts[1]));
                setBattery(Integer.parseInt(parts[2]));
                setTemperature(Integer.parseInt(parts[3]));
                setDirection(parts.length > 4 ? parts[4] : "STRAIGHT");
            } catch (NumberFormatException e) {
                // Mantener valores actuales si hay error de parsing
            }
        }
    }
    
    // Formatear para display
    public String getSpeedDisplay() { return speed + " km/h"; }
    public String getBatteryDisplay() { return battery + "%"; }
    public String getTemperatureDisplay() { return temperature + "°C"; }
}
