#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>

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
    void startServer(int port);

private:
    std::vector<Question> questions;
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
        std::getline(ss, q.question, ','); // Lee la pregunta
        ss >> q.correct_answer;
        ss.ignore();
        for (int i = 0; i < 3; ++i) {
            std::getline(ss, option, ','); // Lee las opciones
            q.options[i] = option;
        }
        questions.push_back(q);
    }
    file.close();
}




void QuizServer::handleClient(int client_socket) {
    char buffer[BUFFER_SIZE];
    int score = 0;

    for (const auto& q : questions) {
        snprintf(buffer, sizeof(buffer), "Pregunta: %s\n1. %s\n2. %s\n3. %s\n",
                 q.question.c_str(), q.options[0].c_str(),
                 q.options[1].c_str(), q.options[2].c_str());
        send(client_socket, buffer, strlen(buffer), 0);
        std::cout << "Enviando pregunta: " << buffer << std::endl;

        memset(buffer, 0, sizeof(buffer)); // Limpia el buffer
        recv(client_socket, buffer, sizeof(buffer), 0);
        int answer = atoi(buffer);
        std::cout << "Respuesta recibida: " << answer << std::endl; // Log
        if (answer == q.correct_answer) {
            score++;
            std::cout << "Respuesta correcta para la pregunta: " << q.question << std::endl;
        } else {
            std::cout << "Respuesta incorrecta para la pregunta: " << q.question << std::endl;
        }
    }

    snprintf(buffer, sizeof(buffer), "Tu puntaje final es: %d de %zu\n", score, questions.size());
    send(client_socket, buffer, strlen(buffer), 0);
    std::cout << "Enviando puntaje final: " << score << " de " << questions.size() << std::endl;
    close(client_socket);
}

void QuizServer::startServer(int port) {
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
        handleClient(client_socket);
    }

    close(server_socket);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cerr << "Uso: " << argv[0] << " -p <puerto> -a <archivo> -c <cantidad>\n";
        return 1;
    }

    int port = atoi(argv[2]);
    QuizServer server;
    server.loadQuestions(argv[4]);
    server.startServer(port);

    return 0;
}
