#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
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

private:
    std::vector<Question> questions;
    std::set<int> connected_clients;
    int minimum_users;

    std::mutex mtx;  // Mutex para proteger secciones críticas
    bool game_started = false;  // Bandera para saber si ya inició el juego
};

void QuizServer::loadQuestions(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        perror("Error al abrir el archivo de preguntas");
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(file, line)) {
        Question q;
        std::stringstream ss(line);
        std::string option;
        std::getline(ss, q.question, ',');
        ss >> q.correct_answer;
        ss.ignore();
        for (int i = 0; i < 3; ++i) {
            std::getline(ss, option, ',');
            q.options[i] = option;
        }
        questions.push_back(q);
    }
    file.close();
}

void QuizServer::handleClient(int client_socket) {
    // Agregar cliente a la lista de clientes conectados
    {
        std::lock_guard<std::mutex> lock(mtx);  // Proteger la inserción con un mutex
        connected_clients.insert(client_socket);
    }

    // Enviar mensaje al cliente que está esperando
    std::string waiting_message = "Esperando a más jugadores para iniciar...\n";
    send(client_socket, waiting_message.c_str(), waiting_message.length(), 0);

    // Esperar hasta alcanzar el número mínimo de usuarios
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (connected_clients.size() >= minimum_users && !game_started) {
                game_started = true;  // Marcar que la partida ha comenzado
                broadcastMessage("¡La partida ha comenzado!\n");
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));  // Esperar un segundo
    }

    // Enviar preguntas y procesar respuestas
    char buffer[BUFFER_SIZE];
    int score = 0;

    for (const auto& q : questions) {
        snprintf(buffer, sizeof(buffer), "Pregunta: %s\n1. %s\n2. %s\n3. %s\n",
                 q.question.c_str(), q.options[0].c_str(),
                 q.options[1].c_str(), q.options[2].c_str());
        send(client_socket, buffer, strlen(buffer), 0);

        recv(client_socket, buffer, sizeof(buffer), 0);
        int answer = atoi(buffer);
        if (answer == q.correct_answer) {
            score++;
        }
    }

    snprintf(buffer, sizeof(buffer), "Tu puntaje final es: %d de %zu\n", score, questions.size());
    send(client_socket, buffer, strlen(buffer), 0);

    close(client_socket);

    // Remover cliente de la lista de conectados al finalizar
    {
        std::lock_guard<std::mutex> lock(mtx);
        connected_clients.erase(client_socket);
    }
}

void QuizServer::broadcastMessage(const std::string& message) {
    for (int client_socket : connected_clients) {
        send(client_socket, message.c_str(), message.length(), 0);
    }
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
    if (argc < 7) { // Asegúrate de que hay suficientes argumentos
        std::cerr << "Uso: " << argv[0] << " -p <puerto> -a <archivo> -c <cantidad> -u <usuarios>\n";
        return 1;
    }

    int port = atoi(argv[2]);
    int min_users = atoi(argv[6]); // Obtener número mínimo de usuarios
    QuizServer server;
    server.loadQuestions(argv[4]);
    server.startServer(port, min_users); // Iniciar el servidor con el número mínimo de usuarios

    return 0;
}
