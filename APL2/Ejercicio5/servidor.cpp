#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

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
    void handleClient(int client_socket, int client_id);
    void startServer(int port, int required_users, int questions_count);

private:
    std::vector<Question> questions;
    std::vector<int> scores;
    std::mutex mtx; // Mutex for thread safety
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

void QuizServer::handleClient(int client_socket, int client_id) {
    char buffer[BUFFER_SIZE];
    int score = 0;

    for (const auto& q : questions) {
        snprintf(buffer, sizeof(buffer), "Pregunta: %s\n1. %s\n2. %s\n3. %s\n",
                 q.question.c_str(), q.options[0].c_str(),
                 q.options[1].c_str(), q.options[2].c_str());
        send(client_socket, buffer, strlen(buffer), 0);
        std::cout << "Enviando pregunta: " << buffer << std::endl;

        memset(buffer, 0, sizeof(buffer)); // Limpia el buffer
        ssize_t recv_size = recv(client_socket, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            std::cout << "Cliente " << client_id << " desconectado\n";
            break; // El cliente se ha desconectado
        }
        
        int answer = atoi(buffer);
        std::cout << "Respuesta recibida de cliente " << client_id << ": " << answer << std::endl; // Log
        if (answer == q.correct_answer) {
            score++;
            std::cout << "Respuesta correcta para la pregunta: " << q.question << std::endl;
        } else {
            std::cout << "Respuesta incorrecta para la pregunta: " << q.question << std::endl;
        }
    }

    {
        std::lock_guard<std::mutex> lock(mtx); // Bloquea el acceso al recurso compartido
        scores[client_id] = score; // Guarda el puntaje del cliente
    }
    
    snprintf(buffer, sizeof(buffer), "Tu puntaje final es: %d de %zu\n", score, questions.size());
    send(client_socket, buffer, strlen(buffer), 0);
    std::cout << "Enviando puntaje final a cliente " << client_id << ": " << score << " de " << questions.size() << std::endl;
    close(client_socket);
}

void QuizServer::startServer(int port, int required_users, int questions_count) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    std::vector<std::thread> client_threads;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, MAX_CLIENTS);

    std::cout << "Servidor esperando conexiones en el puerto " << port << "...\n";

    scores.resize(required_users); // Inicializa el vector de puntajes

    while (client_threads.size() < required_users) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        std::cout << "Cliente conectado\n";

        int client_id = client_threads.size(); // Asigna un ID al cliente
        client_threads.emplace_back(&QuizServer::handleClient, this, client_socket, client_id);
    }

    for (auto& thread : client_threads) {
        thread.join(); // Espera a que todos los hilos terminen
    }

    // Anunciar el ganador
    int winner_id = std::distance(scores.begin(), std::max_element(scores.begin(), scores.end()));
    std::cout << "El ganador es el Cliente " << winner_id << " con un puntaje de " << scores[winner_id] << "!\n";
    close(server_socket);
}

int main(int argc, char *argv[]) {
    if (argc < 9) {
        std::cerr << "Uso: " << argv[0] << " -p <puerto> -u <usuarios> -a <archivo> -c <cantidad>\n";
        return 1;
    }

    int port = atoi(argv[2]);
    int required_users = atoi(argv[4]);
    std::string questions_file = argv[6];
    int questions_count = atoi(argv[8]); // Este parámetro no se utiliza, puedes implementarlo si es necesario

    QuizServer server;
    server.loadQuestions(questions_file);
    server.startServer(port, required_users, questions_count);

    return 0;
}
