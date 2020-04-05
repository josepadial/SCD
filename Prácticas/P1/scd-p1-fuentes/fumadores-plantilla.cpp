#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;


const int NUM_FUMADORES = 3;

Semaphore mostr_vacio=1, ingr_disp[NUM_FUMADORES]={0,0,0};



//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

int producir(){
	int numero;
	numero = rand()%3;
	return numero;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
	while(true){
		int recurso_producido;

    	recurso_producido = producir();

		sem_wait(mostr_vacio);

		string s1[] = {"El estanquero produce tabaco","El estanquero produce papel","El estanquero produce cerillas"};

		cout << s1[recurso_producido] << endl;
		sem_signal(ingr_disp[recurso_producido]);
	}

}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
   		sem_wait(ingr_disp[num_fumador]);
   		string s1[] = { "Retirado el ingrediente tabaco ", "Retirado el ingrediente papel ", "Retirado el ingrediente cerillas "} ;
   		cout << "El fumador: " << num_fumador << s1[num_fumador] << endl;
   		sem_signal(mostr_vacio);
   		fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   thread hebra_estanquero;
   thread fumador[NUM_FUMADORES];

   hebra_estanquero = thread(funcion_hebra_estanquero);
   for(int i=0; i<NUM_FUMADORES; i++){
   		fumador[i] = thread(funcion_hebra_fumador,i);
   }

   hebra_estanquero.join();
   for(int i=0; i<NUM_FUMADORES; i++){
   		fumador[i].join();
   }
}
