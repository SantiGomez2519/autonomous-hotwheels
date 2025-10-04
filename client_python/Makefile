# Makefile para Cliente Python
# Ejecuta el cliente modular con separación de responsabilidades

# Comando Python
PYTHON = python3

# Archivos fuente
SOURCES = vehicle_data.py network_manager.py main.py
MAIN_FILE = main.py

# Regla principal
all:
	@echo "Cliente Python listo para ejecutar"

# Ejecutar el cliente
run: check-deps
	$(PYTHON) $(MAIN_FILE)

# Verificar dependencias
check-deps:
	@echo "Verificando dependencias..."
	@$(PYTHON) -c "import tkinter; print('tkinter: OK')" || (echo "Error: tkinter no disponible" && exit 1)
	@echo "socket: OK (biblioteca estándar)"
	@echo "threading: OK (biblioteca estándar)"
	@echo "datetime: OK (biblioteca estándar)"
	@echo "Dependencias verificadas"

# Mostrar ayuda
help:
	@echo "Makefile para Cliente Python"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "  make          - Verificar que el cliente está listo"
	@echo "  make run      - Ejecutar el cliente"
	@echo "  make check-deps - Verificar dependencias"
	@echo "  make help     - Mostrar esta ayuda"
	@echo "  make compare  - Comparar con versión original"
	@echo ""
	@echo "Módulos del cliente:"
	@echo "  - vehicle_data.py: Modelo de datos del vehículo"
	@echo "  - network_manager.py: Gestión de comunicación de red"
	@echo "  - main.py: Interfaz gráfica de usuario"

# Comparar con versión original
compare:
	@echo "Comparando versiones..."
	@echo "Versión original:"
	@echo "  - client.py: $(shell wc -l client.py 2>/dev/null || echo 'N/A')"
	@echo "  - gui.py: $(shell wc -l gui.py 2>/dev/null || echo 'N/A')"
	@echo "Versión actual:"
	@echo "  - vehicle_data.py: $(shell wc -l vehicle_data.py)"
	@echo "  - network_manager.py: $(shell wc -l network_manager.py)"
	@echo "  - main.py: $(shell wc -l main.py)"

# Regla phony
.PHONY: all run check-deps help compare
