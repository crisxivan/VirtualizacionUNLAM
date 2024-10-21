FROM ubuntu:24.10

# Establecer el directorio de trabajo
WORKDIR /apl

# Instalar g++ y make
RUN apt-get update && \
    apt-get install -y g++ make && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Copiar los archivos al contenedor
COPY . .

# Compilar el proyecto
RUN make

# Comando por defecto para ejecutar el servidor
CMD ["./servidor", "-p", "8080", "-a", "preguntas.txt", "-u", "2", "-c", "3"]


