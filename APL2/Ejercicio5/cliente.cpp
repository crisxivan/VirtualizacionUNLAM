#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 6) {
        std::cerr << "Uso: " << argv[0] << " -n <nickname> -p <puerto> -s <servidor>\n";
        return 1;
    }

    char *nickname = argv[2];
    int port = std::stoi(argv[4]);
    char *server_ip = argv[6];

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
        return 1;
    }

    std::cout << "Conectado al servidor como " << nickname << "\n";

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        
        // Recibiendo datos del servidor
        ssize_t recv_size = recv(client_socket, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            std::cerr << "Error al recibir datos o conexión cerrada\n";
            break;
        }

        // Mostrando el mensaje del servidor
        std::cout << "Mensaje del servidor: " << buffer << "\n";

        // Verificando si el mensaje es una pregunta
        if (strncmp(buffer, "Pregunta:", 8) == 0) {
            std::cout << "Tu respuesta (1-3): ";

            memset(buffer, 0, sizeof(buffer));  // Limpiar el buffer antes de leer la entrada
            fgets(buffer, sizeof(buffer), stdin);

            // Enviando respuesta al servidor
            ssize_t send_size = send(client_socket, buffer, strlen(buffer), 0);
            if (send_size < 0) {
                std::cerr << "Error al enviar respuesta\n";
                break;
            }

            std::cout << "Respuesta enviada: " << buffer;  // Log de la respuesta enviada
        }
    }

    close(client_socket);
    std::cout << "Conexión cerrada\n";  // Log de cierre de conexión
    return 0;
}
