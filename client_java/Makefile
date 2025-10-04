# Makefile para Cliente Java Refactorizado
# Compila el cliente modular con separación de responsabilidades

# Compilador Java
JAVAC = javac
JAVA = java

# Archivos fuente
SOURCES = VehicleData.java NetworkManager.java ClientGUI.java
CLASSES = $(SOURCES:.java=.class)

# Clase principal
MAIN_CLASS = ClientGUI

# Regla principal
all: $(CLASSES)
	@echo "Cliente Java refactorizado compilado exitosamente"

# Compilar archivos Java
%.class: %.java
	$(JAVAC) $<

# Ejecutar el cliente
run: $(CLASSES)
	$(JAVA) $(MAIN_CLASS)

# Limpiar archivos compilados
clean:
	rm -f *.class
	@echo "Archivos de compilación eliminados"

# Mostrar ayuda
help:
	@echo "Makefile para Cliente Java Refactorizado"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "  make          - Compilar el cliente refactorizado"
	@echo "  make run      - Ejecutar el cliente"
	@echo "  make clean    - Eliminar archivos compilados"
	@echo "  make help     - Mostrar esta ayuda"
	@echo "  make compare  - Comparar con versión original"
	@echo ""
	@echo "Módulos del cliente:"
	@echo "  - VehicleData: Modelo de datos del vehículo"
	@echo "  - NetworkManager: Gestión de comunicación de red"
	@echo "  - ClientGUI: Interfaz gráfica de usuario"

# Comparar con versión original
compare: $(CLASSES)
	@echo "Comparando versiones..."
	@echo "Versión original: $(shell wc -l Client.java 2>/dev/null || echo 'N/A')"
	@echo "Versión refactorizada:"
	@echo "  - VehicleData.java: $(shell wc -l VehicleData.java)"
	@echo "  - NetworkManager.java: $(shell wc -l NetworkManager.java)"
	@echo "  - ClientGUI.java: $(shell wc -l ClientGUI.java)"

# Regla phony
.PHONY: all run clean help compare
