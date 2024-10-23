/**
Integrantes del grupo:
-Cespedes, Cristian Ivan -> DNI 41704776
-Gomez, Luciano Dario -> DNI 41572055
-Luna, Leandro Santiago -> DNI 40886200
-Panigazzi, Agustin Fabian -> DNI 43744593
**/

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>

void manejarSenales(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        std::cout << "Proceso " << getpid() << " finalizando correctamente.\n";
        exit(0);
    }
}

void mostrarAyuda() {
    std::cout << "Uso: programa [opciones]\n";
    std::cout << "Opciones:\n";
    std::cout << "  -h       Muestra esta ayuda.\n";
    std::cout << "Este programa crea una jerarquía de procesos con un padre, dos hijos,\n";
    std::cout << "donde uno de los hijos tiene tres nietos (uno de los cuales será un proceso zombie),\n";
    std::cout << "y el otro hijo crea un demonio que permanece en ejecución.\n";
}

void crearNietosHijo1() {
    pid_t nieto1 = fork();
    if (nieto1 == 0) { // Nieto 1
        std::cout << "Soy el nieto 1 con PID " << getpid() << ", mi padre es " << getppid() << "\n";
        pause(); // Se queda en espera
        exit(0);
    }

    pid_t nieto2 = fork();
    if (nieto2 == 0) { // Nieto 2
        std::cout << "Soy el nieto 2 con PID " << getpid() << ", mi padre es " << getppid() << "\n";
        exit(0); // Finaliza normalmente
    }

    pid_t nieto3 = fork();
    if (nieto3 == 0) { // Nieto 3 (zombie)
        std::cout << "Soy el nieto 3 con PID " << getpid() << ", mi padre es " << getppid() << "\n";
        pause(); // Se queda en espera
        exit(0);
    }

    // Espera a que los nietos terminen (menos el zombie)
    waitpid(nieto2, nullptr, 0);
    waitpid(nieto1, nullptr, 0);
}

void crearHijo2() {
    pid_t hijo2 = fork();
    if (hijo2 == 0) { // Hijo 2
        pid_t demonio = fork();
        if (demonio == 0) { // Demonio
            std::cout << "Soy el demonio con PID " << getpid() << ", mi padre es " << getppid() << "\n";
            setsid(); // Crea una nueva sesión
            while (true) {
                // El demonio puede realizar su tarea aquí.
                sleep(1); // Simulación de trabajo
            }
        }
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    // Verificar argumentos
    if (argc > 1 && strcmp(argv[1], "-h") == 0) {
        mostrarAyuda();
        return 0;
    }

    signal(SIGINT, manejarSenales);
    signal(SIGTERM, manejarSenales);

    // Crear el proceso padre
    std::cout << "Soy el proceso padre con PID " << getpid() << "\n";

    // Crear Hijo 1
    pid_t hijo1 = fork();
    if (hijo1 == 0) { // Hijo 1
        std::cout << "Soy el hijo 1 con PID " << getpid() << ", mi padre es " << getppid() << "\n";
        crearNietosHijo1();
        exit(0);
    }

    // Crear Hijo 2
    crearHijo2();

    // Esperar a los hijos
    waitpid(hijo1, nullptr, 0);
    waitpid(-1, nullptr, 0); // Espera al hijo 2

    return 0;
}
