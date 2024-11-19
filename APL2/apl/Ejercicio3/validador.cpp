#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstring>
#include <ctime>
#include <signal.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstdio>

#define FIFO_NAME "/tmp/fifo_huellas"

// Estructura para representar una huella
struct Huella {
    int sensor_id;
    long huella_id;
};

void escribir_log(const std::string& mensaje, const std::string& log_file) {
    std::ofstream log(log_file, std::ios_base::app);
    if (log.is_open()) {
        log << mensaje << std::endl;
        log.close();
    } else {
        std::cerr << "Error al abrir el archivo de log: " << log_file << std::endl;
        exit(1);
    }
}

bool es_valida(long huella_id) {
    return (huella_id >= 1000000000 && huella_id <= 1999999999);
}

void manejar_signal(int signal) {
    if (signal == SIGTERM || signal == SIGINT) {
        std::cout << "Proceso de validador finalizado." << std::endl;
        // Eliminar el FIFO al terminar
        unlink(FIFO_NAME);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Uso incorrecto. Se debe especificar el archivo de log y FIFO." << std::endl;
        exit(1);
    }

    std::string log_file = argv[1];
    std::string fifo_name = argv[2];

    // Configurar el proceso como demonio
    if (fork() != 0) {
        return 0;  // El proceso padre termina, dejando el demonio corriendo
    }
    setsid();  // Crear una nueva sesión para el demonio

    // Registrar el manejador de señales
    signal(SIGTERM, manejar_signal);
    signal(SIGINT, manejar_signal);

    // Crear el FIFO si no existe
    if (mkfifo(fifo_name.c_str(), 0666) == -1) {
        std::cerr << "Error al crear el FIFO: " << fifo_name << std::endl;
        exit(1);
    }

    // Abrir el FIFO para lectura
    int fifo = open(fifo_name.c_str(), O_RDONLY);
    if (fifo == -1) {
        std::cerr << "Error al abrir el FIFO: " << fifo_name << std::endl;
        exit(1);
    }

    while (true) {
        Huella huella;
        ssize_t bytes_leidos = read(fifo, &huella, sizeof(Huella));
        if (bytes_leidos > 0) {
            // Obtener la fecha y hora actual
            std::time_t t = std::time(nullptr);
            std::tm* tm_info = std::localtime(&t);
            char fecha_hora[20];
            strftime(fecha_hora, sizeof(fecha_hora), "%Y-%m-%d %H:%M:%S", tm_info);

            std::string resultado = es_valida(huella.huella_id) ? "Huella válida" : "Huella no válida";

            // Escribir en el log
            std::string mensaje = std::string(fecha_hora) + " Sensor: " + std::to_string(huella.sensor_id)
                                  + " Huella ID: " + std::to_string(huella.huella_id) + " - " + resultado;
            escribir_log(mensaje, log_file);
        }
    }

    close(fifo);
    return 0;
}
