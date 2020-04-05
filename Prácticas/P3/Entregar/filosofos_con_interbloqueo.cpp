/*
	Autor: Jose Antonio Padial Molina
	Asignatura: Sistemas Concurrentes y Distribuidos
	Fecha: Diciembre 2018
	Curso: 2018/2019
*/

#include <iostream>
#include <thread>
#include <random>
#include <chrono>
#include <mpi.h>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

const int numero_filosofos = 5, numero_procesos = 2*numero_filosofos;

//Numero aleatorio en el rango min-max incluidos
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void funcion_filosofos(int id){
	int id_tenedor_izq = (id+1) % numero_procesos,
		id_tenedor_dcha = (id+numero_procesos-1) % numero_procesos,
		peticion;

	while(true){
		cout << "Filosofo numero: " << id << " solicita tenedor izquierdo: " <<
			id_tenedor_izq << endl;
		MPI_Ssend(&peticion, 1, MPI_INT, id_tenedor_izq, 0, MPI_COMM_WORLD);

		cout << "Filosofo numero: " << id << " solicita tenedor derecho: " <<
			id_tenedor_dcha << endl;
		MPI_Ssend(&peticion, 1, MPI_INT, id_tenedor_dcha, 0, MPI_COMM_WORLD);

		cout << "Filosofo numero: " << id << " comienza a comer" << endl;
		sleep_for( milliseconds( aleatorio<10,100>() ) );

		cout << "Filosofo numero: " << id << " suelta tenedor izquierdo: " <<
			id_tenedor_izq << endl;
		MPI_Ssend(&peticion, 1, MPI_INT, id_tenedor_izq, 0, MPI_COMM_WORLD);

		cout << "Filosofo numero: " << id << " suelta tenedor derecho: " <<
			id_tenedor_dcha << endl;
		MPI_Ssend(&peticion, 1, MPI_INT, id_tenedor_dcha, 0, MPI_COMM_WORLD);

		cout << "Filosofo numero: " << id << " comienza a pensar" << endl;
		sleep_for( milliseconds( aleatorio<10,100>() ) );
	}
}

void funcion_tenedores(int id){
	int valor, id_filosofo;
	MPI_Status estado;

	while(true){
		MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado);
		id_filosofo = estado.MPI_SOURCE;
		cout << "Tenedor numero: " << id << " ha sido cogido por el filosofo numero: " <<
			id_filosofo << endl;

		MPI_Recv(&valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado);
		cout << "Tenedor numero: " << id << " ha sido liberado por el filosofo numero: " <<
			id_filosofo << endl;
	}
}

int main(int argc, char *argv[]){
	int id_propio, num_procesos_actuales;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
	MPI_Comm_size(MPI_COMM_WORLD, & num_procesos_actuales);

	if(numero_procesos == num_procesos_actuales){
		if(id_propio%2 == 0)
			funcion_filosofos(id_propio);
		else
			funcion_tenedores(id_propio);
	}
	else{
		if(id_propio == 0){
			cout << "El numero de procesos esperados es: " << numero_procesos << endl
             << "el numero de procesos en ejecucion es: " << num_procesos_actuales << endl
             << "(programa abortado)" << endl ;
		}
	}

	MPI_Finalize();
	return 0;
}