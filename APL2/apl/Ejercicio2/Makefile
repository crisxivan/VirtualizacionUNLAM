# Definir el compilador y las opciones
CXX = g++

# Nombre del ejecutable
TARGET = Ejercicio2

# Archivos fuente
SRC = Ejercicio2.cpp

# Regla por defecto
all: $(TARGET)

# Regla para crear el ejecutable
$(TARGET): $(SRC)
	$(CXX) -o $(TARGET) $(SRC)

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGET)
