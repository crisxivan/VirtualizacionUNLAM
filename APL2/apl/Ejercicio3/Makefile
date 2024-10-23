# Definimos el compilador y las opciones
CXX = g++
CXXFLAGS = -Wall -g

# Nombres de los ejecutables
TARGETS = lector validador

# Archivos fuente
SRC_LECTOR = lector.cpp
SRC_VALIDADOR = validador.cpp

# Regla por defecto
all: $(TARGETS)

# Regla para crear el ejecutable lector
lector: $(SRC_LECTOR)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Regla para crear el ejecutable validador
validador: $(SRC_VALIDADOR)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGETS)
