/*
==============================================================================================================
APL2 Ejercicio 4
Grupo 
Integrantes:


==============================================================================================================
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/shm.h> 	
#include <fcntl.h>	
#include <semaphore.h> 
#include <signal.h>
#include <string>
#include <iostream>
#include <fstream>
#include <limits>

#define SEGMENTO_ID	504


using namespace std;
typedef struct
{
    char usuario[50];
    char pregunta[250];
    char respuesta1[250];
    char respuesta2[250];
    char respuesta3[250];
    int rta_final;
    bool fin_partida;
}Mensaje;

void mostrarAyuda();
void clearConsole();


bool fin=false;
bool sen_servidor = true;

//variables globales para cerrar cliente
sem_t *leer;
sem_t *leerRta;
sem_t *escribir;
Mensaje *area_compartida;

int main(int argc, char* argv[]){	

	signal(SIGINT, SIG_IGN);
	Mensaje men;
    if (argc == 1){
        cout << "----------Error de parametros----------" <<endl;
        cout << "\nPara mostrar la ayuda, ejecute:" << endl;
	    cout << "- ./clienteComp -h" << endl;
        exit(1);
    }

    if((strcasecmp("-n",argv[1]) == 0 || strcasecmp("--nickname",argv[1]) == 0) && argc==3)
    {
        strcpy(men.usuario,argv[2]);
    }
    else if((strcasecmp("-h", argv[1]) == 0) && argc==2)
    {
        mostrarAyuda();
        return 0;
    }
    else
    {
        cout << "Comandos incorrectos, ingrese \"./clienteComp -h\" para ver la ayuda" << endl;
        exit(1);
    }

    char archivoServidor[19] = "/tmp/servidor.txt";
    char archivoCliente[18] = "/tmp/Cliente.txt";
    
    ifstream arch_cliente(archivoCliente);
    if (!arch_cliente.is_open()) 
    {
        ofstream clienteRegistrado(archivoCliente);
        if(!clienteRegistrado.is_open())
        {
            cout << "No se pudo registrar el cliente" << endl;
            exit(1);
        }
        clienteRegistrado.close();
    }else
    {
        cout << "error: no se puede conectar mas de un cliente al servidor" << endl;
        exit(1);
    }
    arch_cliente.close();


    ifstream arch_servidor(archivoServidor);
    if(!arch_servidor)
    {
        cout << "El servidor no se encuentra en linea, no es posible conectarse" << endl;
        remove(archivoCliente);
        exit (1);
    }
    arch_servidor.close();

    // MANEJADOR DE SEÑAL
    signal(SIGINT, SIG_IGN);
    
	leer =	sem_open("/leer",O_CREAT);
    leerRta = sem_open("/leerRta", O_CREAT);
	escribir = sem_open("/escribir",O_CREAT);
	
	int shmid = shmget(SEGMENTO_ID, sizeof(Mensaje), IPC_CREAT | 0666);
	
	area_compartida = (Mensaje*)shmat( shmid, NULL, 0);

    while(!fin)
    {
        sem_wait(leer);
        men = *area_compartida;
        sem_post(escribir);

        if(!men.fin_partida)
        {
            cout << men.pregunta << endl << "1)" << men.respuesta1 << endl << "2)" << men.respuesta2 << endl << "3)" << men.respuesta3 << endl;
            do{
                cout << "ingrese el numero de respuesta correcta (entre 1 y 3)";
                cin >> men.rta_final;
                if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
                cout << "Entrada no válida. Por favor, ingrese un número entre 1 y 3." << endl;
                }
            }while(men.rta_final < 1 || men.rta_final > 3);

            sem_wait(escribir);
            *area_compartida = men;
            sem_post(leerRta);
            
            clearConsole();
        }else
        {
            cout << "La partida ha finalizado" << endl << "Su puntuacion final es: "<< men.rta_final << endl;
            fin =true;
        }

    }

	shmdt( &area_compartida );	

    sem_close( leerRta );
	sem_close( leer );
	sem_close( escribir );

    remove("/tmp/Cliente.txt");
    puts("finalizando usuario.."); 

    return 0;
}


/**
 * Limpia la pantalla y mueve el cursor a la esquina superior izquierda.
 * Utiliza el escape ANSI para limpiar la pantalla y mover el cursor.
 */
void clearConsole() {
    cout << "\033[2J\033[1;1H"; 
}

void mostrarAyuda()
{
	puts("-------------- AYUDA --------------");
	puts("\n Este programa es el juego preguntados.");
    puts("Para conectarse: ");
    puts("./ClienteComp -n <nombre usuario>");
    puts("./ClienteComp --nickname <nombre usuario>");
}

