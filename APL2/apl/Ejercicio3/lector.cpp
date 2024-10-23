/**
Integrantes del grupo:
-Cespedes, Cristian Ivan -> DNI 41704776
-Gomez, Luciano Dario -> DNI 41572055
-Luna, Leandro Santiago -> DNI 40886200
-Panigazzi, Agustin Fabian -> DNI 43744593
**/

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#define NOMBRE_FIFO "/tmp/fifo_huellas"

void enviarHuella(int numeroSensor, int intervalo, int cantidadMensajes, const std::string &archivoIDs) {
    std::ifstream ids(archivoIDs);
    if (!ids.is_open()) {
        std::cerr << "Error abriendo el archivo de IDs." << std::endl;
        return;
    }

    int fd = open(NOMBRE_FIFO, O_WRONLY);
    if (fd == -1) {
        std::cerr << "Error abriendo el FIFO para escritura." << std::endl;
        return;
    }

    std::string linea;
    for (int i = 0; i < cantidadMensajes && getline(ids, linea); ++i) {
        std::string mensaje = std::to_string(numeroSensor) + " " + linea;
        write(fd, mensaje.c_str(), mensaje.size()); // Cambiar a mensaje.size()
        std::this_thread::sleep_for(std::chrono::seconds(intervalo));
    }

    close(fd);
    ids.close();
}


void mostrarAyuda() {
    std::cout << "Uso: lector -n <numero_sensor> -s <segundos> -m <cantidad_mensajes> -i <archivo_ids>" << std::endl;
    std::cout << "  -n, --numero  Número del sensor." << std::endl;
    std::cout << "  -s, --segundos Intervalo en segundos para el envío del mensaje." << std::endl;
    std::cout << "  -m, --mensajes Cantidad de mensajes a enviar." << std::endl;
    std::cout << "  -i, --ids    Archivo con los números de ID a informar desde el lector." << std::endl;
    std::cout << "  -h, --help   Muestra esta ayuda." << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        mostrarAyuda();
        return 1;
    }

    int numeroSensor = -1, segundos = -1, cantidadMensajes = -1;
    std::string archivoIDs;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            mostrarAyuda();
            return 0;
        }
        if (strcmp(argv[i], "-n") == 0 && (i + 1) < argc) {
            numeroSensor = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0 && (i + 1) < argc) {
            segundos = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-m") == 0 && (i + 1) < argc) {
            cantidadMensajes = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-i") == 0 && (i + 1) < argc) {
            archivoIDs = argv[++i];
        }
    }

    if (numeroSensor < 0 || segundos < 0 || cantidadMensajes < 0 || archivoIDs.empty()) {
        std::cerr << "Parámetros inválidos." << std::endl;
        mostrarAyuda();
        return 1;
    }

    enviarHuella(numeroSensor, segundos, cantidadMensajes, archivoIDs);
    return 0;
}
