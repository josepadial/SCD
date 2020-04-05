// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2. SU LIFO PRODCONS
//
// archivo: productor-consumidor_SU_LIFO.cpp
//
// Autor: Jose Antonio Padial Molina
//
// -----------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"

using namespace std;
using namespace HM ;

//**********************************************************************
// variables compartidas

constexpr int num_items = 40;    // número de items
unsigned  cont_prod[num_items], // contadores de verificación: producidos
          cont_cons[num_items]; // contadores de verificación: consumidos
mutex m;     // mutex de escritura en pantalla
const int num_productor = 2, num_consumidor = 4;

unsigned cont_producidos[num_consumidor] = {0};

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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato( int num_prod )
{
   int dato = num_prod * (num_items/num_productor) + cont_producidos[num_prod];
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   m.lock();
   cout << "producido: " << dato << endl << flush ;
   m.unlock();
   cont_prod[dato] ++ ;
   cont_producidos[num_prod]++;

   return dato;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
  if(num_items <= dato){
    cout << " dato === " << dato << ", num_items == " << num_items << endl ;
    assert( dato < num_items );
  }
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   m.lock();
   cout << "                  consumido: " << dato << endl ;
   m.unlock();
}

//----------------------------------------------------------------------

void ini_contadores()
{
  for( unsigned i = 0 ; i < num_items ; i++ )
   {  cont_prod[i] = 0 ;
      cont_cons[i] = 0 ;
   }
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version LIFO, semántica SU, varios prod. y varios cons.

class ProdConsdSuLifo:public HoareMonitor{
  private:
    static const int num_celdas_total = 10;
    int buffer[num_celdas_total], primera_libre;
    CondVar puede_producir;
    CondVar puede_escribir;
  public:
    ProdConsdSuLifo();
    int leer(int num);
    void escribir(int valor, int num);
};
// -----------------------------------------------------------------------------

ProdConsdSuLifo::ProdConsdSuLifo(){
  primera_libre = 0;
  puede_producir = newCondVar();
  puede_escribir = newCondVar();
}
// -----------------------------------------------------------------------------

int ProdConsdSuLifo::leer(int num){
  cout << "Llega hebra consumidora " <<setw(2) << num << endl ;

  if ( primera_libre == 0 )
      puede_escribir.wait( );

  assert( 0 < primera_libre  );
  primera_libre-- ;
  const int valor = buffer[primera_libre] ;

  puede_producir.signal();

  cout << "              Sale hebra consumidora " <<setw(2) << num << endl;

  return valor ;
}
// -----------------------------------------------------------------------------

void ProdConsdSuLifo::escribir(int valor, int num){
  cout << "Llega hebra productora " <<setw(2) << num << endl ;

  if ( primera_libre == num_celdas_total )
      puede_producir.wait();

  assert( primera_libre < num_celdas_total );

  buffer[primera_libre] = valor ;
  primera_libre++ ;

  puede_escribir.signal();

  cout << "              Sale hebra productora " <<setw(2) << num << endl;
}

//----------------------------------------------------------------------

void funcion_hebra_productora( MRef<ProdConsdSuLifo> monitor , int num)
{
   for( unsigned i = 0 ; i < num_items/num_productor ; i++ )
   {
      int valor = producir_dato(num) ;
      monitor->escribir( valor , num);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora( MRef<ProdConsdSuLifo> monitor , int num)
{
   for( unsigned i = 0 ; i < num_items/num_consumidor ; i++ )
   {
      int valor = monitor->leer( num );
      consumir_dato( valor ) ;
   }
}
//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (Monitor SU, buffer LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

  MRef<ProdConsdSuLifo> monitor = Create<ProdConsdSuLifo>();

  thread hebra_productora[num_productor];
  for( unsigned i = 0 ; i < num_productor ; i++ )
    hebra_productora[i] = thread( funcion_hebra_productora, monitor, i);

  thread hebra_consumidora[num_consumidor];
  for( unsigned i = 0 ; i < num_consumidor ; i++ )
    hebra_consumidora[i] = thread( funcion_hebra_consumidora, monitor, i);

  for( unsigned i = 0 ; i < num_productor ; i++ )
      hebra_productora[i].join();

  for( unsigned i = 0 ; i < num_consumidor ; i++ )
      hebra_consumidora[i].join();

   test_contadores();
}