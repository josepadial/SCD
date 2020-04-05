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

const int
	num_filosofos = 5, num_procesos_efectivos = 2*num_filosofos,
	num_procesos_esperando = num_procesos_efectivos+1,
	id_camarero = num_procesos_efectivos;

const int etiq_levantarse = 1, etiq_sentarse = 0;

template< int min, int max > int aleatorio(){
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void funcion_filosofos(int id){
	int id_tenedor_izq = (id+1) % num_procesos_efectivos,
		id_tenedor_dcha = (id+num_procesos_efectivos-1) % num_procesos_efectivos,
		peticion;

	while(true){
		cout << "Filosofo numero: " << id << " solicita permiso para sentarse en la mesa: " << endl;
		MPI_Ssend(&peticion, 1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD);

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

		cout << "Filosofo numero: " << id << " solicita permiso para levantarse" << endl;
		MPI_Ssend(&peticion, 1, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD);

		cout << "Filosofo numero: " << id << " comienza a pensar" << endl;
		sleep_for( milliseconds( aleatorio<10,100>() ) );
	}
}

void funcion_camarero(){
	int num_filosofos_sentados=0,
		peticion, id_filosofo,
		etiq_aceptable;

	MPI_Status estado;

	while(true){
		if(num_filosofos_sentados < num_filosofos - 1)
			etiq_aceptable = MPI_ANY_TAG;
		else
			etiq_aceptable = etiq_levantarse;

		MPI_Recv( &peticion, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado );
		id_filosofo = estado.MPI_SOURCE;

		switch(estado.MPI_TAG){
			case etiq_levantarse:
				cout << "\tFilósofo " << id_filosofo << " se levanta de la mesa" << endl;
				num_filosofos_sentados--;
				break;
			case etiq_sentarse:
				cout << "\tFilósofo " << id_filosofo << " se sienta a la mesa" << endl;
				num_filosofos_sentados++;
				break;
		}

		cout << "\t --- Actualmente hay " << num_filosofos_sentados << " filósofos sentados --- " << endl;
	}
}

void funcion_tenedores(int id){
	int valor, id_filosofo;
	MPI_Status estado;

	while(true){
		MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado );
		id_filosofo = estado.MPI_SOURCE;

		cout <<"Tenedor " <<id <<" ha sido cogido por filosofo " <<id_filosofo <<endl;

    	MPI_Recv( &valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado );

    	cout <<"Tenedor "<< id << " ha sido liberado por filosofo " <<id_filosofo <<endl ;
	}
}

int main(int argc, char* argv[]){
	int id_propio, num_procesos_actual;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
	MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

	if(num_procesos_esperando == num_procesos_actual){
		if(id_propio == id_camarero)
			funcion_camarero();
		else if(id_propio %2 == 0)
			funcion_filosofos(id_propio);
		else
			funcion_tenedores(id_propio);
	}
	else{
		if(id_propio == 0){
			cout << "El numero de procesos esperados es: " << num_procesos_esperando << endl
             << "el numero de procesos en ejecucion es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
		}
	}
	MPI_Finalize();
	return 0;
}