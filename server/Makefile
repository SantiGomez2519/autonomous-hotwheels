# Makefile para el Servidor de Telemetría Vehículo Autónomo
# Compila el servidor modular con soporte para hilos (pthread)

# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lpthread

# Nombre del ejecutable
TARGET = server

# Source files (consolidated version)
SOURCES = server.c socket_manager.c vehicle.c client_protocol.c
OBJECTS = $(SOURCES:.c=.o)

# Regla principal
all: $(TARGET)

# Compile the consolidated server
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)
	@echo "Consolidated server compiled successfully: $(TARGET)"

# Compilar archivos objeto
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean compiled files
clean:
	rm -f $(TARGET) $(OBJECTS)
	@echo "Compiled files removed"

# Instalar el servidor (copiar a /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Servidor instalado en /usr/local/bin"

# Desinstalar el servidor
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Servidor desinstalado"

# Ejecutar el servidor con parámetros por defecto
run: $(TARGET)
	./$(TARGET) 8080 server.log

# Ejecutar el servidor en modo debug
debug: $(TARGET)
	gdb ./$(TARGET) 8080 server.log

# Mostrar ayuda
help:
	@echo "Makefile para Servidor de Telemetría"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "  make          - Compilar el servidor"
	@echo "  make clean    - Eliminar archivos compilados"
	@echo "  make run      - Ejecutar servidor (puerto 8080)"
	@echo "  make debug    - Ejecutar con gdb"
	@echo "  make install  - Instalar en /usr/local/bin"
	@echo "  make uninstall- Desinstalar"
	@echo "  make help     - Mostrar esta ayuda"
	@echo "  make compare  - Comparar versiones"
	@echo ""
	@echo "Uso del servidor:"
	@echo "  ./server <puerto> <archivo_log>"
	@echo "  Ejemplo: ./server 8080 server.log"
	@echo ""
	@echo "Módulos del servidor:"
	@echo "  - socket_manager: Gestión de sockets"
	@echo "  - client_manager: Gestión de clientes"
	@echo "  - vehicle: Estado del vehículo"
	@echo "  - logger: Sistema de logging"
	@echo "  - protocol: Procesamiento de comandos"

# Verificar dependencias del sistema
check-deps:
	@echo "Verificando dependencias..."
	@which gcc > /dev/null || (echo "Error: gcc no encontrado" && exit 1)
	@echo "gcc: OK"
	@echo "pthread: OK (incluido en glibc)"
	@echo "Dependencias verificadas"

# Crear directorio de logs si no existe
setup:
	mkdir -p logs
	@echo "Directorio de logs creado"

# Ejecutar con valgrind para detectar memory leaks
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) 8080 server.log

# Compilar con optimizaciones
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)
	@echo "Versión release compilada"

# Compilar con información de debug
debug-build: CFLAGS += -g -DDEBUG
debug-build: $(TARGET)
	@echo "Versión debug compilada"

# Comparar con versión original
compare: $(TARGET)
	@echo "Comparando versiones..."
	@echo "Versión original: $(shell wc -l server.c 2>/dev/null || echo 'N/A')"
	@echo "Versión actual: $(shell wc -l server.c)"
	@echo "Módulos adicionales:"
	@echo "  - socket_manager.c: $(shell wc -l socket_manager.c)"
	@echo "  - client_manager.c: $(shell wc -l client_manager.c)"
	@echo "  - vehicle.c: $(shell wc -l vehicle.c)"
	@echo "  - logger.c: $(shell wc -l logger.c)"
	@echo "  - protocol.c: $(shell wc -l protocol.c)"

# Regla phony
.PHONY: all clean install uninstall run debug help check-deps setup valgrind release debug-build compare
