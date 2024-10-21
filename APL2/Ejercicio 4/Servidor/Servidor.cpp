/*
==============================================================================================================
APL2 Ejercicio 4
Grupo 
Integrantes:
    - Agustín Fabian Panigazzi


==============================================================================================================
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include <sys/ipc.h> 	//Biblioteca para los flags IPC_ 
#include <sys/shm.h> 	//Para memoria compartida SYSTEM-V	
#include <semaphore.h>	//Para semáforos POSIX
#include <fcntl.h>		//Para utilizar los flags O_
#include <signal.h>
#include <iostream>
#include <string.h>
#include <list>
#include <string>
#include <fstream>
#include <sstream>


#define SIN_MEM             0
#define TODO_OK             1
#define DUPLICADO           2
#define SEGMENTO_ID	    504 //La clave también puede ser creada con ftok()
#define REGISTROS 	1

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


bool fin = false;
void manejador(int);

sem_t *leer, *escribir, *leerRta;
int shmid;
Mensaje *area_compartida;

void finalizarServidor();

// Ayuda
void mostrarAyuda();

int main(int argc, char* argv[]){	

    string archivo;
    int cantidad=0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--archivo") == 0) {
            if (i + 1 < argc) {
                archivo = argv[i + 1];
                i++; // Saltar el siguiente argumento
            } else {
                cout << "Error: Se requiere un argumento para -a/--archivo\n" << endl;
                exit(1);
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cantidad") == 0) {
            if (i + 1 < argc) {
                cantidad = atoi(argv[i + 1]); // Convertir a entero
                i++; // Saltar el siguiente argumento
            } else {
                cout << "Error: Se requiere un argumento para -c/--cantidad\n" << endl;
                exit(1);
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            mostrarAyuda();
            return 0;
        } else {
            cout << "Opción desconocida: " << argv[i] << endl;
            exit(1);
        }
    }
    if(archivo.length() == 0 || cantidad == 0)
    {
        puts("error en los parametros");
        exit(1);
    }

    char archivoServidor[100] = "/tmp/servidor.txt";
    //SI YA HAY UNA INSTANCIA DE SERVIDOR, TERMINA EL PROGRAMA
    if(access(archivoServidor, F_OK) == 0)
    {
        puts("YA HAY UNA INSTANCIA DEL PROCESO SERVIDOR!");
        exit(1);
    }

    ofstream servidor(archivoServidor);
    if(!servidor)
    {
        cout << "No se pudo registrar el servidor" << endl;
        exit(1);
    }

    //CIERRO EL ARCHIVO
    servidor.close();

    // DEMONIO
    signal(SIGUSR1, manejador); //Se asocia la señal SIGUSR1 con el manejador propio
    pid_t pid = fork();

    //Si hay error de creación del hijo, finalizar
    if (pid < 0)
        return 1;

    //Si es el proceso padre, finalizar, así dejamos huérfano al hijo
    if (pid > 0){
        printf("PID del demonio: %d\n", pid);
        return 0;
    }

    //En este punto solo está el huérfano, que será el demonio

       // Creamos una nueva sesión para el demonio, básicamente con esto no quedará tomando la terminal /
    pid_t sid = setsid();
    if (sid < 0) {
        printf("Error al ejecutar setsid\n");
        exit(1);
    }

	//Creación de los semáforos
    leer = sem_open("/leer", O_CREAT | O_EXCL, 0666, 0);
    leerRta			=	sem_open( "/leerRta",		O_CREAT | O_EXCL,	0666,	0);
	escribir	=	sem_open( "/escribir",	O_CREAT | O_EXCL,	0666,	1); 

	
	signal(SIGINT, SIG_IGN);
    signal(SIGUSR1,manejador);

	Mensaje men;
	//Creación de memoria compartida
	shmid = shmget(	SEGMENTO_ID, sizeof(Mensaje), IPC_CREAT | 0666);

	//Vincular la memoria compartida a una variable local
	area_compartida = (Mensaje*)shmat(shmid,NULL,0);
    if (area_compartida == (Mensaje *) -1) {
    perror("Error en shmat");
    exit(1);
    }

    int cantidad_preg = 0, correctas = 0;
    string linea, rta_correc;

    ifstream preguntas(archivo);
    if(!preguntas.is_open())
    {
        cout << "No se pudo abrir el archivo de preguntas" << endl;
        exit(1);
    }
    while(!fin)
    {   
        if(cantidad_preg<cantidad)
        {   
            men.fin_partida = false;
            if (!getline(preguntas, linea))
            {
                // Si llegamos al final del archivo, reiniciamos el puntero
                preguntas.clear(); // Limpiar el estado (EOF)
                preguntas.seekg(0); // Volver al inicio del archivo
                continue; // Comenzar de nuevo el ciclo
            }
            istringstream iss(linea);

            iss.getline( men.pregunta,256, ',');
            getline(iss, rta_correc, ',');
            iss.getline(men.respuesta1,256, ',');
            iss.getline(men.respuesta2,256, ',');
            iss.getline(men.respuesta3,256);

            sem_wait(escribir);
            *area_compartida = men;
            sem_post(leer);
            
            sem_wait(leerRta);
            men = *area_compartida;
            sem_post(escribir);

            if(stoi(rta_correc) == men.rta_final)
                correctas++;

            cantidad_preg++;

        }
        else //final de la partida
        {
            men.rta_final = correctas;
            men.fin_partida  = true;

            sem_wait(escribir);
            *area_compartida = men;
            sem_post(leer);
            
            correctas = 0;
            cantidad_preg = 0;
        }
    }

    finalizarServidor();

    return 0;
}

/**
 * Finaliza el servidor y borra la memoria compartida y los semaforos
 *
 * Verifica si hay un cliente conectado y si no es así, borra la memoria compartida
 * y los semáforos y cierra los archivos asociados.
 */
void finalizarServidor(){
    char archivoCliente[18] = "/tmp/Cliente.txt";
    ifstream archivo(archivoCliente);
    if(!archivo)
    {
        shmdt(&area_compartida);
        // Marcar la memoria compartida para borrar
        shmctl(shmid, IPC_RMID, NULL);

        // Cierre de los semáforos
        sem_close(leerRta);
        sem_close(leer);
        sem_close(escribir);

        // Marcar los semáforos para destruirlos
        sem_unlink("/leerRta");
        sem_unlink("/leer");
        sem_unlink("/escribir");

        // BORRO ARCHIVOS
        remove("/tmp/servidor.txt");
        fin=true;
    }else
        puts("el servidor no puede detenerse, ya que, un cliente se encuentra jugando una partida");
}

// demonio
void manejador( int sig ){
    finalizarServidor();
    if(fin)
        exit(0);
    return;
}

/**
 * Muestra la ayuda para el programa
 *
 * Se muestra la ayuda para el usuario, explicando el funcionamiento del
 * programa y los comandos disponibles para el servidor y el cliente.
 */
void mostrarAyuda()
{
	puts("-------------- AYUDA --------------");
	puts(" \n  Este programa es un chat tipo mailbox a la antigua\n "
		"Para ello se crea un servidor y un cliente que se comunican a traves de memoria compartida." );
	puts("Comandos para el Servidor:");
	puts("- 1. ./servidorComp ");
    puts("Para terminarlo");
	puts("kill -SIGUSR1 Pid_Demonio");

}