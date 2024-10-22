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
#include <condition_variable>

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
    std::mutex mtx;  // Mutex para proteger datos compartidos
    std::condition_variable cv;  // Para sincronizar conexiones y finalización
    int connected_clients = 0;  // Contador de clientes conectados
    int finished_clients = 0;   // Contador de clientes que han terminado
    bool game_started = false;  // Indica si el juego comenzó
    bool game_finished = false;  // Indica si el juego finalizó
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

void QuizServer::handleClient(int client_socket, int client_id) {
    {
        std::unique_lock<std::mutex> lock(mtx);
        // Esperar hasta que se conecten todos los clientes necesarios
        cv.wait(lock, [this] { return game_started; });
    }

    char buffer[BUFFER_SIZE];
    int score = 0;

    // Enviar todas las preguntas al cliente
    for (const auto& q : questions) {
        snprintf(buffer, sizeof(buffer), 
                 "Pregunta: %s\n1. %s\n2. %s\n3. %s\n", 
                 q.question.c_str(), q.options[0].c_str(), 
                 q.options[1].c_str(), q.options[2].c_str());
        send(client_socket, buffer, strlen(buffer), 0);

        memset(buffer, 0, sizeof(buffer));  // Limpiar el buffer
        ssize_t recv_size = recv(client_socket, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            std::cout << "Cliente " << client_id << " desconectado\n";
            {
                std::lock_guard<std::mutex> lock(mtx);
                // Elimina el puntaje del cliente
                if (client_id < scores.size()) {
                    scores.erase(scores.begin() + client_id);
                }
                connected_clients--;
            }
            close(client_socket);
            return;  // Termina si el cliente se desconecta
        }

        int answer = atoi(buffer);
        if (answer == q.correct_answer) {
            score++;
        }
    }

    // Guardar el puntaje del cliente en la lista de scores
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (client_id >= scores.size()) {
            scores.resize(client_id + 1, 0);  // Redimensiona e inicializa en 0
        }
        scores[client_id] = score;
        finished_clients++;
        std::cout << " Aguarde a que los demas terminen " << "...\n";
        std::cout << " puntaje "<< scores[client_id] << "...\n";
    }

    // Notificar que un cliente ha terminado
    cv.notify_all();

    // Esperar a que todos los clientes terminen
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return finished_clients == connected_clients; });

    // Enviar el ganador a todos los clientes
    // int winner_id = *std::max_element(scores.begin(), scores.end());
    // int winner_id = std::distance(scores.begin(), 
    //                                std::max_element(scores.begin(), scores.end()));

    int winner_id = 0;
    int max_score = scores[0];

    // std::cout << " size " << scores.size()<< "...\n";
    // Bucle para encontrar el mayor puntaje
    for (int i = 0; i < scores.size(); i++) {
        if (scores[i] > max_score) {
            max_score = scores[i];
            winner_id = i;
            // std::cout << " entro " << "...\n";
        }
    }
    // std::cout << " winner_id " << winner_id << "...\n";
    // std::cout << " max_score " << max_score << "...\n";

    // Obtener el puntaje del ganador
    int winner_score = scores[winner_id];
    
    // std::cout << " winner_id " << winner_id  << "...\n";
    // std::cout << " score " << scores[0] << "...\n";
    // std::cout << " score " << scores[1] << "...\n";
    // std::cout << " score " << scores[2] << "...\n";

    snprintf(buffer, sizeof(buffer), 
             "El ganador es el Cliente %d con un puntaje de %d!\n", 
             winner_id, winner_score);
    send(client_socket, buffer, strlen(buffer), 0);

    close(client_socket);
    // Reseteo las variables
    scores.clear();
    connected_clients = 0;
    finished_clients  = 0;
    game_started      = false;
    game_finished     = true;
}

void QuizServer::startServer(int port, int required_users, int questions_count) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    std::vector<std::thread> client_threads;

    // Crear y configurar el socket del servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, MAX_CLIENTS);

    std::cout << "Servidor esperando conexiones en el puerto " << port << "...\n";
    scores.resize(required_users);  // Inicializar el vector de puntajes

    // Aceptar conexiones hasta alcanzar el número requerido de usuarios
    while (client_threads.size() < MAX_CLIENTS) {
        
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, 
                               (struct sockaddr *)&client_addr, &addr_size);
        std::cout << "Cliente conectado\n";

        {
            std::lock_guard<std::mutex> lock(mtx);
            connected_clients++;
            if (connected_clients >= required_users) {
                std::cout << " JUEGO INICIADO " << "...\n";
                game_started = true;
                cv.notify_all();  // Notificar que el juego puede comenzar
            }

            
        }

        int client_id = client_threads.size();  // Asignar ID al cliente
        client_threads.emplace_back(&QuizServer::handleClient, this, 
                                    client_socket, client_id);

        // if( game_finished == true ) {
        //     // Esperar a que todos los hilos terminen
        //     for (auto& thread : client_threads) {
        //         if (thread.joinable()) {
        //             thread.join();
        //         }
        //          std::cout << "hilo b\n";
        //     }

        //      game_finished = false;
        // }
    }
    
    for (auto& thread : client_threads) {
        thread.join();
    }

    close(server_socket);
}

void showUsage(const char *program_name) {
    std::cerr << "Uso: " << program_name 
              << " -p <puerto> -u <usuarios> -a <archivo> -c <cantidad>\n";
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 9) {
        showUsage(argv[0]);
    }

    int port = 0, required_users = 0, questions_count = 0;
    std::string questions_file;

    // Parseo y validación de los parámetros
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-p") == 0) {
            port = std::stoi(argv[i + 1]);
            if (port <= 0 || port > 65535) {
                std::cerr << "Error: El puerto debe estar entre 1 y 65535.\n";
                exit(1);
            }
        } else if (strcmp(argv[i], "-u") == 0) {
            required_users = std::stoi(argv[i + 1]);
            if (required_users <= 0) {
                std::cerr << "Error: El número de usuarios debe ser mayor que 0.\n";
                exit(1);
            }
        } else if (strcmp(argv[i], "-a") == 0) {
            questions_file = argv[i + 1];
            if (questions_file.empty()) {
                std::cerr << "Error: Debes proporcionar un archivo de preguntas válido.\n";
                exit(1);
            }
        } else if (strcmp(argv[i], "-c") == 0) {
            questions_count = std::stoi(argv[i + 1]);
            if (questions_count <= 0) {
                std::cerr << "Error: La cantidad de preguntas debe ser mayor que 0.\n";
                exit(1);
            }
        } else {
            showUsage(argv[0]);
        }
    }

    QuizServer server;
    server.loadQuestions(questions_file);
    while ( true )
    {
        server.startServer(port, required_users, questions_count);
    }
    
    server.startServer(port, required_users, questions_count);

    return 0;
}