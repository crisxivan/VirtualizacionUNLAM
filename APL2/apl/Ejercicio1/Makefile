# Definimos el compilador y las opciones
CXX = g++
CXXFLAGS = -Wall -g

# Nombre del ejecutable
TARGET = ejercicio1

# Archivos fuente
SRC = ejercicio1.cpp

# Regla por defecto
all: $(TARGET)

# Regla para crear el ejecutable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGET)
