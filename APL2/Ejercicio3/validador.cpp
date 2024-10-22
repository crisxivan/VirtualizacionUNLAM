/**
Integrantes del grupo:
-Cespedes, Cristian Ivan -> DNI 41704776
-Gomez, Luciano Dario -> DNI 41572055
-Luna, Leandro Santiago -> DNI 40886200
-Panigazzi, Agustin Fabian -> DNI 43744593
**/

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include <cstdlib>
#include <cstring>

#define NOMBRE_FIFO "/tmp/fifo_huellas"

bool enEjecutivo = true;

// Simulación de validación de huella
bool esHuellaValida(const std::string &idHuella) {
    return (idHuella.length() % 2 == 0); // Ejemplo: válidas si la longitud es par
}

void manejarSenal(int senal) {
    enEjecutivo = false;
}

void registrarMensaje(const std::string &mensaje, const std::string &archivoLog) {
    std::ofstream log(archivoLog, std::ios::app);
    if (log.is_open()) {
        time_t ahora = time(0);
        char* dt = ctime(&ahora);
        std::string marcaTemporal(dt);
        marcaTemporal.pop_back(); // Eliminar el carácter '\n'
        
        log << marcaTemporal << ": " << mensaje << std::endl;
        log.close();
    }
}

void validarHuella(const std::string &idSensor, const std::string &idHuella, const std::string &archivoLog) {
    std::string resultado = "Sensor " + idSensor + (esHuellaValida(idHuella) ? " validó huella: " : " NO validó huella: ") + idHuella;
    registrarMensaje(resultado, archivoLog);
}

void mostrarAyuda() {
    std::cout << "Uso: validador -l <archivo_log>" << std::endl;
    std::cout << "  -l, --log     Archivo de log donde se escribirán los mensajes." << std::endl;
    std::cout << "  -h, --help    Muestra esta ayuda." << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        mostrarAyuda();
        return 1;
    }

    std::string archivoLog;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            mostrarAyuda();
            return 0;
        }
        if (strcmp(argv[i], "-l") == 0 && (i + 1) < argc) {
            archivoLog = argv[++i];
        }
    }

    signal(SIGINT, manejarSenal);
    signal(SIGTERM, manejarSenal);

    // Crear FIFO
    if (mkfifo(NOMBRE_FIFO, 0666) == -1) {
        return 1;
    }

    // Abrir FIFO para lectura
    std::ifstream fifo(NOMBRE_FIFO);
    if (!fifo.is_open()) {
        return 1;
    }

    while (enEjecutivo) {
        std::string linea;
        if (getline(fifo, linea)) {
            if (!linea.empty()) {
                size_t separador = linea.find(' ');
                if (separador != std::string::npos) {
                    std::string idSensor = linea.substr(0, separador);
                    std::string idHuella = linea.substr(separador + 1);
                    validarHuella(idSensor, idHuella, archivoLog);
                }
            }
        }
    }

    fifo.close();
    unlink(NOMBRE_FIFO); // Eliminar FIFO al finalizar

    return 0;
}
