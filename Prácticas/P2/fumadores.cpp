// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2. Introducción a los monitores en C++11.
//
// archivo: fumadores.cpp
// Ejemplo de un monitor en C++11 con semántica SC, para el problema
// de los fumadores, con tres fumadores.
//
// Historial:
// Creado por Jose Antonio Padial Molina el 03/10/2018
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"
using namespace HM;
using namespace std ;

static const int num_fumadores = 3;

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

//-------------------------------------------------------------------------
// Funcion para producir un valor aleatorio entre 0 y 2

int producir(){
	int numero;
	numero = rand()%3;
	return numero;
}

//-------------------------------------------------------------------------
// Clase para el monitor de Fumador

class Monitorfumador : public HoareMonitor{
	private:
		int ingrediente_disponible;
		CondVar cond_fumadores[num_fumadores], cond_estanquero;
		bool vacio, lleno;
	public:
		Monitorfumador();
		void PonerIngrediente(int ingrediente);
		void ObtenerIngrediente(int ingrediente);
		void EsperarRecogida();
};

Monitorfumador::Monitorfumador(){
	ingrediente_disponible = -1;
	for(int i=0; i<num_fumadores; i++)
		cond_fumadores[i] = newCondVar();

	cond_estanquero = newCondVar();
}

void Monitorfumador::PonerIngrediente(int ingrediente){
	ingrediente_disponible = ingrediente;
	cond_fumadores[ingrediente].signal();
}

void Monitorfumador::ObtenerIngrediente(int ingrediente){
	if(ingrediente_disponible != ingrediente)
		cond_fumadores[ingrediente].wait();

	ingrediente_disponible = -1;
	cond_estanquero.signal();
}

void Monitorfumador::EsperarRecogida(){
	if(ingrediente_disponible != -1)
		cond_estanquero.wait(); // Si hay un ingrediente esperamos a que se retire para poner otro
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(MRef<Monitorfumador> monitor){
	while(true){
		int ingrediente = producir();
		cout << "\tEstanquero: " << ingrediente << endl;
		monitor -> PonerIngrediente(ingrediente);
		monitor -> EsperarRecogida();
	}
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador

void funcion_hebra_fumador(MRef<Monitorfumador> monitor, int num_fumador){
	while(true){
		monitor -> ObtenerIngrediente(num_fumador);
		fumar(num_fumador);
	}
}

//----------------------------------------------------------------------

int main(){
	auto monitor = Create<Monitorfumador>();

	thread hebra_estanquero (funcion_hebra_estanquero, monitor);

	thread hebra_fumador[num_fumadores];
	for(int i=0; i<num_fumadores; i++)
		hebra_fumador[i] = thread (funcion_hebra_fumador, monitor, i);

	for(int i=0; i<num_fumadores; i++)
		hebra_fumador[i].join();
}