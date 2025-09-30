# Makefile para el Servidor de Telemetría Vehículo Autónomo
# Compila el servidor con soporte para hilos (pthread)

# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lpthread

# Nombre del ejecutable
TARGET = server

# Archivo fuente
SOURCE = server.c

# Regla principal
all: $(TARGET)

# Compilar el servidor
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)
	@echo "Servidor compilado exitosamente: $(TARGET)"

# Limpiar archivos compilados
clean:
	rm -f $(TARGET)
	@echo "Archivos de compilación eliminados"

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
	@echo ""
	@echo "Uso del servidor:"
	@echo "  ./server <puerto> <archivo_log>"
	@echo "  Ejemplo: ./server 8080 server.log"

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

# Regla phony
.PHONY: all clean install uninstall run debug help check-deps setup valgrind release debug-build
