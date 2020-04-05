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
	num_prod=4, num_cons=5,
	num_procesos=num_cons+num_prod+1,
	num_items=100, tam_vector=40,
	item_cada_prod=num_items/num_prod,
	item_cada_cons=num_items/num_cons,
	id_buffer=num_prod;

const int 
	etiqueta_productor=0,
	etiqueta_consumidor=1;

//Numero aleatorio en el rango min-max incluidos
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//Producir un valor en orden 1,2,3,... con retardo aleatorio
int producir(int num_productor){
	static int contador = num_productor*item_cada_prod;

	sleep_for(milliseconds(aleatorio<10,100>()));

	contador++;

	cout << "Productor numero: " << num_productor << " ha producido el valor: " <<
		contador << endl << flush;

	return contador;
}

//Consume dato con retardo
void consumir(int valor_cons, int num_consumidor){
	sleep_for(milliseconds(aleatorio<110,200>()));

	cout << "\t\tEl consumidor numero: " << num_consumidor << " ha consumido el valor: " <<
		valor_cons << endl << flush; 
}

//Funcion del productor
void funcion_productor(int num_productor){
	for(unsigned i=0; i<item_cada_prod; i++){
		int valor_prod = producir(num_productor);

		cout << "Productor numero: " << num_productor << " va a enviar: " <<
		valor_prod << endl << flush;

		MPI_Ssend(&valor_prod, 1, MPI_INT, id_buffer,etiqueta_productor, MPI_COMM_WORLD);
	}
}

//Funcion consumidor
void funcion_consumidor(int num_consumidor){
	int peticion, valor_rec;
	MPI_Status estado;

	for(unsigned i=0; i<item_cada_prod; i++){
		//Envia una peticion para consumir un dato
		MPI_Ssend(&peticion, 1, MPI_INT, id_buffer, etiqueta_consumidor, MPI_COMM_WORLD);

		//Recive un dato para consumir
		MPI_Recv(&valor_rec, 1, MPI_INT, id_buffer, etiqueta_productor, MPI_COMM_WORLD, &estado);
		cout << "\t\tEl consumidor numero: " << num_consumidor << " ha recivido el valor: " <<
		valor_rec << endl << flush;

		consumir(valor_rec, num_consumidor);
	}
}

//Funcion para el buffer
void funcion_buffer(){
	int buffer[tam_vector], valor,
		primera_libre = 0,
		primera_ocupada = 0,
		num_celda_ocupadas = 0,
		etiqueta_aceptable;
	MPI_Status estado;

	for(unsigned i=0; i<num_items*2; i++){
		if(num_celda_ocupadas == 0)
			etiqueta_aceptable = etiqueta_productor;
		else if(num_celda_ocupadas == tam_vector)
			etiqueta_aceptable = etiqueta_consumidor;
		else
			etiqueta_aceptable = MPI_ANY_TAG;

		MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiqueta_aceptable, MPI_COMM_WORLD, &estado);

		switch(estado.MPI_TAG){
			case etiqueta_productor:
				buffer[primera_libre] = valor;
				primera_libre = (primera_libre + 1) % tam_vector;
				num_celda_ocupadas++;
				cout << "\tBuffer ha recibido: " << valor << endl;
            	break;

        	case etiqueta_consumidor:
        	valor = buffer[primera_ocupada];
        	primera_ocupada = (primera_ocupada + 1) % tam_vector;
            num_celda_ocupadas--;
            cout << "\tBuffer va a enviar: " << valor << endl;
            MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, etiqueta_productor, MPI_COMM_WORLD );
            break;
		}
	}
}

int main(int argc, char *argv[]){
	int id_propio, num_procesos_actuales;

	if (num_items % num_prod != 0 || num_items % num_cons != 0)
   {
     cout << "error: num_items debe ser múltiplo de num_consumidor y de num_productor " << endl;
     return 1;
   }

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
   MPI_Comm_size(MPI_COMM_WORLD, &num_procesos_actuales);

   if(num_procesos == num_procesos_actuales){
   		if(id_propio < id_buffer)
   			funcion_productor(id_propio);
   		else if(id_propio == id_buffer)
   			funcion_buffer();
   		else
   			funcion_consumidor(id_propio);
   }
   else{
   		if(id_propio == 0){
   			cout << "error: el número de procesos esperados es " << num_procesos
             << ", pero el número de procesos en ejecución es " << num_procesos_actuales << endl;
   		}
   }

   MPI_Finalize();
   return 0;
}
