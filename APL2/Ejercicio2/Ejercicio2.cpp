#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <filesystem>
#include <unistd.h>
#include <getopt.h>
#include <atomic>

using namespace std;
namespace fs = std::filesystem;

mutex mtx;
vector<fs::path> archivos;
int archivo_actual = 0;
atomic<bool> cadena_encontrada(false); 

void buscarCadena(int thread_id, const string& cadena) {
    while (true) {
        mtx.lock();
        if (archivo_actual >= archivos.size()) {
            mtx.unlock();
            break;
        }
        fs::path archivo = archivos[archivo_actual];
        archivo_actual++;
        mtx.unlock();

        ifstream file(archivo);
        if (!file.is_open()) {
            cerr << "Error al abrir el archivo: " << archivo << endl;
            continue;
        }

        string linea;
        int num_linea = 0;

        while (getline(file, linea)) {
            num_linea++;
            if (linea.find(cadena) != string::npos) {
                mtx.lock();
                cout << "Thread " << thread_id << ": " << archivo << ": línea " << num_linea << endl;
                cadena_encontrada = true;
                mtx.unlock();
            }
        }
        file.close();
    }
}

int main(int argc, char* argv[]) {
    int num_threads = 0;
    string directorio;
    string cadena_buscar;

    struct option long_options[] = {
        {"threads", required_argument, 0, 't'},
        {"directorio", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "t:d:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                num_threads = stoi(optarg);
                break;
            case 'd':
                directorio = optarg;
                break;
            case 'h':
                cout << "Uso: " << argv[0] << " -t <num_threads> -d <directorio> <cadena_a_buscar>\n";
                cout << "  -t, --threads <nro>        Cantidad de threads a ejecutar.\n";
                cout << "  -d, --directorio <path>    Ruta del directorio a analizar.\n";
                cout << "  -h, --help                 Muestra la ayuda.\n";
                return 0;
            default:
                cerr << "Opción inválida. Usa -h o --help para ver las opciones disponibles.\n";
                return 1;
        }
    }

    if (optind >= argc) {
        cerr << "Error: Debes proporcionar la cadena a buscar.\n";
        return 1;
    }
    cadena_buscar = argv[optind];

    if (num_threads <= 0 || directorio.empty()) {
        cerr << "Error: debes ingresar un número válido de threads y un directorio.\n";
        return 1;
    }

    for (const auto& entry : fs::directory_iterator(directorio)) {
        if (entry.is_regular_file()) {
            archivos.push_back(entry.path());
        }
    }

    if (archivos.empty()) {
        cerr << "Error: No se encontraron archivos en el directorio especificado.\n";
        return 1;
    }

    vector<thread> threads;
    for (int i = 0; i < num_threads; i++) {
        threads.push_back(thread(buscarCadena, i + 1, cadena_buscar));
    }

    for (int i = 0; i < num_threads; i++) {
        threads[i].join();
    }

    if (!cadena_encontrada) {
        cout << "No se encontró la cadena \"" << cadena_buscar << "\" en ningún archivo.\n";
    } else {
        cout << "Búsqueda completada.\n";
    }

    return 0;
}
