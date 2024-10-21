#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <arpa/inet.h>
#include <vector>
#include <set>
#include <thread>
#include <mutex>

#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

struct Question {
    std::string question;
    int correct_answer;
    std::string options[3];
};

class QuizServer {
public:
    void loadQuestions(const std::string& filename);
    void handleClient(int client_socket);
    void startServer(int port, int min_users);
    void broadcastMessage(const std::string& message);
    void startGame();

private:
    std::vector<Question> questions;
    std::set<int> connected_clients;
    std::mutex mtx; // Para controlar el acceso concurrente
    int minimum_users;
    bool game_started = false;
    int current_question_index = 0; // Para rastrear la pregunta actual
};

void QuizServer::loadQuestions(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        perror("Could not open questions file");
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(file, line)) {
        Question q;
        std::stringstream ss(line);
        std::string option;
        std::getline(ss, q.question, ','); // Leer la pregunta
        ss >> q.correct_answer; // Leer la respuesta correcta
        ss.ignore(); // Ignorar la coma
        for (int i = 0; i < 3; ++i) {
            std::getline(ss, option, ','); // Leer las opciones
            q.options[i] = option; // Asignar a las opciones
        }
        questions.push_back(q);
    }
    file.close();
}

void QuizServer::handleClient(int client_socket) {
    {
        std::lock_guard<std::mutex> lock(mtx); // Bloquear acceso al conjunto de clientes
        connected_clients.insert(client_socket);
    }

    std::string waiting_message = "Esperando a más jugadores para iniciar...\n";
    send(client_socket, waiting_message.c_str(), waiting_message.length(), 0);

    // Esperar hasta que se alcance el número mínimo de usuarios
    while (true) {
        std::lock_guard<std::mutex> lock(mtx); // Bloquear acceso a los clientes
        if (connected_clients.size() >= minimum_users) {
            break; // Salir si se alcanzó el número mínimo
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Esperar un segundo
    }

    // Una vez que se alcance el número mínimo, iniciar el juego
    startGame();

    // Manejar preguntas y respuestas
    int score = 0;
    char buffer[BUFFER_SIZE];

    while (true) {
        {
            std::lock_guard<std::mutex> lock(mtx); // Bloquear acceso a la pregunta actual
            if (current_question_index >= questions.size()) {
                break; // Salir si no hay más preguntas
            }

            const Question& q = questions[current_question_index];
            snprintf(buffer, sizeof(buffer), "Pregunta: %s\n1. %s\n2. %s\n3. %s\n",
                     q.question.c_str(), q.options[0].c_str(),
                     q.options[1].c_str(), q.options[2].c_str());
            send(client_socket, buffer, strlen(buffer), 0);
        }

        // Esperar la respuesta del cliente
        memset(buffer, 0, sizeof(buffer)); // Limpiar el buffer antes de recibir
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0) {
            std::cerr << "Error al recibir datos del cliente\n";
            break; // Salir si hay un error
        }

        buffer[bytes_received] = '\0'; // Asegurarse de que el buffer se termine correctamente
        int answer = atoi(buffer); // Convertir respuesta a entero
        std::cout << "Respuesta recibida: " << answer << std::endl; // Mostrar la respuesta recibida

        // Validar la respuesta
        {
            std::lock_guard<std::mutex> lock(mtx); // Bloquear acceso a las preguntas
            if (answer == questions[current_question_index].correct_answer) {
                score++;
            }
            current_question_index++; // Avanzar a la siguiente pregunta
        }
    }

    snprintf(buffer, sizeof(buffer), "Tu puntaje final es: %d de %zu\n", score, questions.size());
    send(client_socket, buffer, strlen(buffer), 0);
    
    {
        std::lock_guard<std::mutex> lock(mtx); // Bloquear acceso al conjunto de clientes
        connected_clients.erase(client_socket);
    }
    close(client_socket);
}


void QuizServer::broadcastMessage(const std::string& message) {
    for (int client_socket : connected_clients) {
        send(client_socket, message.c_str(), message.length(), 0);
    }
}

void QuizServer::startGame() {
    game_started = true;
    std::string start_message = "¡La partida ha comenzado!\n";
    broadcastMessage(start_message);
}

void QuizServer::startServer(int port, int min_users) {
    minimum_users = min_users; // Inicializar el número mínimo de usuarios
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, MAX_CLIENTS);

    std::cout << "Servidor esperando conexiones en el puerto " << port << "...\n";

    while (true) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        std::cout << "Cliente conectado\n";
        std::thread(&QuizServer::handleClient, this, client_socket).detach(); // Manejar cliente en un hilo separado
    }

    close(server_socket);
}

int main(int argc, char *argv[]) {
    if (argc < 7) {
        std::cerr << "Uso: " << argv[0] << " -p <puerto> -a <archivo> -c <cantidad> -u <usuarios>\n";
        return 1;
    }

    int port = atoi(argv[2]);
    int min_users = atoi(argv[6]);
    QuizServer server;
    server.loadQuestions(argv[4]);
    server.startServer(port, min_users);

    return 0;
}
