#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void showUsage(const char *program_name) {
    std::cerr << "Uso: " << program_name 
              << " -n <nickname> -p <puerto> -s <servidor>\n";
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        showUsage(argv[0]);
    }

    char *nickname = nullptr;
    int port = 0;
    char *server_ip = nullptr;

    // Parsear los parámetros
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-n") == 0) {
            nickname = argv[i + 1];
        } else if (strcmp(argv[i], "-p") == 0) {
            port = std::stoi(argv[i + 1]);
            if (port <= 0 || port > 65535) {
                std::cerr << "Error: El puerto debe ser un número entre 1 y 65535.\n";
                exit(1);
            }
        } else if (strcmp(argv[i], "-s") == 0) {
            server_ip = argv[i + 1];
            if (inet_addr(server_ip) == INADDR_NONE) {
                std::cerr << "Error: Dirección IP del servidor inválida.\n";
                exit(1);
            }
        } else {
            showUsage(argv[0]);
        }
    }

    if (!nickname) {
        std::cerr << "Error: El nickname no puede estar vacío.\n";
        exit(1);
    }

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Intentando crear el socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "Error al crear el socket\n";
        return 1;
    }

    // Configuración de la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    std::cout << "Intentando conectar al servidor " << server_ip << " en el puerto " << port << "...\n";

    // Intentando conectarse al servidor
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Conexión fallida");
        close(client_socket);
        return 1;
    }

    std::cout << "Conectado al servidor como " << nickname << "\n";

    bool juego_en_progreso = true;  // Bandera para mantener el bucle del juego

    while (juego_en_progreso) {
        memset(buffer, 0, sizeof(buffer));

        // Recibiendo datos del servidor
        ssize_t recv_size = recv(client_socket, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            std::cerr << "Error al recibir datos o conexión cerrada\n";
            break;
        }

        // Mostrar mensaje recibido del servidor
        std::cout << "Mensaje del servidor: " << buffer << "\n";

        // Verificar si es una pregunta
        if (strncmp(buffer, "Pregunta:", 8) == 0) {
            std::cout << "Tu respuesta (1-3): ";

            memset(buffer, 0, sizeof(buffer));  // Limpiar buffer antes de leer la entrada
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;  // Quitar el salto de línea

            // Enviar respuesta al servidor
            ssize_t send_size = send(client_socket, buffer, strlen(buffer), 0);
            if (send_size < 0) {
                std::cerr << "Error al enviar respuesta\n";
                break;
            }
        } 
        // Verificar si es el mensaje de fin del juego
        else if (strncmp(buffer, "El ganador es", 13) == 0) {
            std::cout << "¡Juego terminado! " << buffer << "\n";
            juego_en_progreso = false;  // Salir del bucle
        }
    }

    std::cout << "Desconectando...\n";
    close(client_socket);
    return 0;
}