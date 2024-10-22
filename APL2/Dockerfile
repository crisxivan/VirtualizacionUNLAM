# Usar la imagen base Ubuntu 24.10
FROM ubuntu:24.10

WORKDIR /apl

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libpthread-stubs0-dev \
    && apt-get clean

# Copia los archivos del proyecto al contenedor
COPY ./ /apl/

# Compila cada ejercicio
RUN cd ejercicio1 && make
RUN cd ejercicio2 && make
RUN cd ejercicio3 && make
RUN cd ejercicio4 && make
RUN cd ejercicio5 && make

# para mantener el contenedor activo
CMD ["bash"]