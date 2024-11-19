#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <getopt.h>
#include <cstdio>

#define FIFO_NAME "/tmp/fifo_huellas"

struct Huella {
    int sensor_id;
    long huella_id;
};

void mostrar_ayuda() {
    std::cout << "Uso: lector [opciones]" << std::endl;
    std::cout << "Opciones:" << std::endl;
    std::cout << "  -l, --log       Archivo de log donde se irán escribiendo los mensajes (Requerido)" << std::endl;
    std::cout << "  -n, --numero    Número del sensor (Requerido)" << std::endl;
    std::cout << "  -s, --segundos  Intervalo en segundos para el envío del mensaje (Requerido)" << std::endl;
    std::cout << "  -m, --mensajes  Cantidad de mensajes a enviar (Requerido)" << std::endl;
    std::cout << "  -i, --ids       Archivo con los números de ID a informar desde el lector (Requerido)" << std::endl;
    std::cout << "  -h, --help      Muestra esta ayuda" << std::endl;
}

void enviar_huella(int sensor_id, long huella_id, const std::string& fifo_name) {
    int fifo = open(fifo_name.c_str(), O_WRONLY);
    if (fifo == -1) {
        std::cerr << "Error al abrir el FIFO: " << fifo_name << std::endl;
        exit(1);
    }

    Huella huella = {sensor_id, huella_id};
    ssize_t bytes_escritos = write(fifo, &huella, sizeof(Huella));
    if (bytes_escritos == -1) {
        std::cerr << "Error al escribir en el FIFO: " << fifo_name << std::endl;
        exit(1);
    }

    close(fifo);
}

int main(int argc, char* argv[]) {
    if (argc < 11) {
        mostrar_ayuda();
        return 1;
    }

    std::string log_file;
    int sensor_id = 0;
    int segundos = 0;
    int cantidad_mensajes = 0;
    std::string archivo_ids;

    struct option opciones[] = {
        {"log", required_argument, nullptr, 'l'},
        {"numero", required_argument, nullptr, 'n'},
        {"segundos", required_argument, nullptr, 's'},
        {"mensajes", required_argument, nullptr, 'm'},
        {"ids", required_argument, nullptr, 'i'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "l:n:s:m:i:h", opciones, nullptr)) != -1) {
        switch (opt) {
            case 'l':
                log_file = optarg;
                break;
            case 'n':
                sensor_id = std::stoi(optarg);
                break;
            case 's':
                segundos = std::stoi(optarg);
                break;
            case 'm':
                cantidad_mensajes = std::stoi(optarg);
                break;
            case 'i':
                archivo_ids = optarg;
                break;
            case 'h':
                mostrar_ayuda();
                return 0;
            default:
                mostrar_ayuda();
                return 1;
        }
    }

    // Verificar si todos los parámetros requeridos están presentes
    if (log_file.empty() || sensor_id == 0 || segundos == 0 || cantidad_mensajes == 0 || archivo_ids.empty()) {
        std::cerr << "Faltan parámetros requeridos." << std::endl;
        mostrar_ayuda();
        return 1;
    }

    // Abrir archivo de IDs
    std::ifstream archivo(archivo_ids);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo de IDs: " << archivo_ids << std::endl;
        return 1;
    }

    // Crear el FIFO
    if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST) {
        std::cerr << "Error al crear el FIFO: " << FIFO_NAME << std::endl;
        return 1;
    }

    std::string linea;
    int contador = 0;
    while (std::getline(archivo, linea) && contador < cantidad_mensajes) {
        try {
            long huella_id = std::stol(linea);
            enviar_huella(sensor_id, huella_id, FIFO_NAME);
        } catch (const std::exception& e) {
            std::cerr << "Error al procesar el ID de la huella: " << linea << std::endl;
            continue;
        }

        contador++;
        std::this_thread::sleep_for(std::chrono::seconds(segundos));
    }

    archivo.close();
    unlink(FIFO_NAME);  // Eliminar el FIFO temporal
    return 0;
}
