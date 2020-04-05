// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2. Introducción a los monitores en C++11.
//
// archivo: barbero_durmiente.cpp
// Ejemplo de un monitor en C++11 con semántica SC, para el problema
// del barbero durmiente, para 10 clientes.
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

static const int num_clientes = 10;

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

void esperaFueraBarberia(){
	// calcular milisegundos aleatorios de duración de la espera
   chrono::milliseconds espera( aleatorio<200,2000>() );

   // espera bloqueada un tiempo igual a ''espera' milisegundos
   this_thread::sleep_for( espera );
}

//-------------------------------------------------------------------------

void cortarPeloCliente(){
	// calcular milisegundos aleatorios de duración de la espera
   chrono::milliseconds espera( aleatorio<200,2000>() );

   // espera bloqueada un tiempo igual a ''espera' milisegundos
   this_thread::sleep_for( espera );
}

//-------------------------------------------------------------------------
// Clase para el monitor de Barberia

class Barberia : public HoareMonitor{
	private:
		CondVar barbero, silla, clientes;
	public:
		Barberia();
		void cortarPelo(int cliente);
		void siguienteCliente();
		void finCliente();
};

Barberia::Barberia(){
	barbero = newCondVar();
	silla = newCondVar();
	clientes = newCondVar();
}

void Barberia::cortarPelo(int cliente){
	cout << "LLega el cliente: " << cliente << endl;

	if(clientes.empty()){
		if(!silla.empty()){
			cout << "El cliente: " << cliente << " espera al barbero" << endl;
			clientes.wait();
		}
		else{
			cout << "El cliente: " << cliente << " despierta al barbero" << endl;
			barbero.signal();
		}
	}
	else{
		cout << "El cliente: " << cliente << " se pone a la cola" << endl;
		clientes.wait();
	}

	cout << "El cliente: " << cliente << " se sienta en la silla del barbero" << endl;
	silla.wait();
}

void Barberia::siguienteCliente(){
	if(clientes.empty()&&silla.empty()){
		cout << "\t\t\tEl babero se duerme, no hay clientes" << endl;
		barbero.wait();
	}
	else{
		if(silla.empty()){
			cout << "\t\t\tEl babero llama al siguiente cliente" << endl;
			clientes.signal();
		}
	}
}

void Barberia::finCliente(){
	cout << "\t\t\tEl babero termina de con el cliente" << endl;
	silla.signal();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del barbero

void funcion_hebra_barbero(MRef<Barberia> monitor){
	while(true){
		monitor -> siguienteCliente();
		cortarPeloCliente();
		monitor -> finCliente();
	}
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del cliente

void funcion_hebra_cliente(MRef<Barberia> monitor, int num_clie){
	while(true){
		monitor -> cortarPelo(num_clie);
		cout << "El cliente: " << num_clie << "espera fuera" << endl;
		esperaFueraBarberia();
	}
}

//----------------------------------------------------------------------

int main(){
	auto monitor = Create<Barberia>();

	thread hebra_barbero(funcion_hebra_barbero, monitor);

	thread hebra_cliente[num_clientes];
	for(int i=0; i<num_clientes; i++)
		hebra_cliente[i] = thread (funcion_hebra_cliente, monitor, i);

	for(int i=0; i<num_clientes; i++)
		hebra_cliente[i].join();
}