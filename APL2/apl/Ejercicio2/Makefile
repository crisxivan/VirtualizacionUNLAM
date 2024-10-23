# Definir el compilador y las opciones
CXX = g++
CXXFLAGS = -Wall -g -std=c++11 -pthread

# Nombre del ejecutable
TARGET = ejercicio2

# Archivos fuente
SRC = ejercicio2.cpp

# Regla por defecto
all: $(TARGET)

# Regla para crear el ejecutable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGET)
