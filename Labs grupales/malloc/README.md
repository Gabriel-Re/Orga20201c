Empezando el Readme

Lista implicita
Abra dos estructuras en esta implementacion: Una para un bloque libre y otra para un bloque ocupado.

El diseño elegido para el trabajo en cuanto a la lista, es la lista implicita, a continuación se dará detalles de como estan compuestos los bloques y las primitivas que utilizamos.

struct bloque_libre {
  uint64_t header;            -----> Contiene el tamaño del bloque y esta ocupado (tamaño = 8 bytes)
  uint64_t footer;            -----> Contiene el tamaño del bloque y esta ocupado  (tamaño = 8 bytes)
};

Tamaño, en bytes, minimo de un bloque libre = 16


|Bloque libre|
 |----------------------------------------------|
 |HEADER    Tamaño bloque |     |     |alloc bit| (64 bits)
 |----------------------------------------------|
 |                                              |
 |                                              |
 |                                              |
 |----------------------------------------------|
 |FOOTER: Tamaño bloque   |     |     |alloc bit| (64 bits)
 |----------------------------------------------|

Tamaño minimo de un bloque libre =  16 bytes

struct bloque_ocupado{
  uint64_t header;          ------> Contiene el tamaño del bloque y esta ocupado (tamaño = 8 bytes)
  uint64_t footer;          ------> Contiene el tamaño del bloque y esta ocupado  (tamaño = 8 bytes)
  void* payload;            ------> Contiene el puntero de la informacion que se esta guardando en memoria ( 16 bytes)
};
|Bloque ocupado|
|----------------------------------------------|
|HEADER    Tamaño bloque |     |     |alloc bit|
|----------------------------------------------|
|               Data                           | <--- Puntero que devuelve mm_malloc y mm_free lo libera
|----------------------------------------------|
|               Data                           |
|----------------------------------------------|
|FOOTER: Tamaño bloque   |     |     |alloc bit|
|----------------------------------------------|

Tamaño minimo de un bloque ocupado = 32 bytes

|Heap|

 bloque_epilogo (tamanio 0);        ------> Tope del heap
Static void*  heap_listp;           ------> Bloque prologo del heap

|----------------------------------------------|
|                                              |
|                  STACK                       |
|                                              |
|----------------------------------------------|
|                                              |
|                                              |
|                                              |
|----------------------------------------------|  <----- Tope del heap (program break)
|                                              |
|                                              |  ^
|                  HEAP                        |  |
|                                              |  |  Crece para arriba el heap
|                                              |  
|----------------------------------------------|  <----- Comienzo del heap (Heap_start)
|                Globales, codigo              |
|----------------------------------------------|


            |prologue block|   Regular blocks|epilogue block|
            |-----------------------------------------------|
|START HEAP|| 8/1  |  8/1  |.......|.........|          |0/1|
            |-----------------------------------------------|
                  ^
                  |
                  Static char* heap_listp


Eso a cuanto la estructuración de los datos. Se debe incluir, además, una descripción (en pseudo-código es suficiente) de las primitivas necesarias para el manejo de estos datos. En particular:

primitivas para conocer el tamaño de un bloque, y su estado
primitivas para encontrar los bloques siguiente y anterior

static const uint64_t WSIZE = 8;
static const uint64_t DSIZE = 16;

static uint64_t obtener_word(void* header){
   return (* (uint64_t*)  header);
}

// Cambia el header de un bloque de memoria

static void escribir_header( void* header, uint64_t valor){
   (* (uint64_t*) header) = valor;
}

//Obtiene el tamaño de un bloque

static void* obtener_header(void* block);


static uint64_t block_size(void* puntero){
  return obtener_word(puntero) & ~0xf;
}

// Devuelve un bool diciendo si un bloque esta asignado o no

static bool is_allocated( void* puntero){
  return (obtener_word(puntero) & 0x01);
}

// Devuelve un puntero al header de un bloque actual

static void* obtener_header(void* block){
  return ((char*) block - WSIZE);
}

// Devuelve un puntero al footer del bloque pasado

static void* obtener_footer( void* block){
  return (  (char*) (block) + block_size(obtener_header((char*)block)) - DSIZE);
}

// Devuelve un puntero al proximo bloque

static void* proximo_bloque( void* block){
  return ( ( (char*) (block) + block_size(obtener_header((char*)block) )) - WSIZE);
}

// Devuelve un puntero al bloque anterior

static void* bloque_anterior( void* block){
  return ((char*) (block) - block_size( ((char*)block - DSIZE)  ));
}

Lista de inconsistencias que pueden llegar a ocurrir

1) Invariantes en el tamaño del header y el footer. (Tamaño de bloque y bit de alocacion)
    -Obtenemos los bits de header y del footer y mediante las primitivas de block size y mascaras que utilizan esas funciones junto con la funcion is_allocated comparamos ambos parte para ver si coinciden

2) Alineacion de los bloques
    -Hacemos division con modulo con un DSIZE para ver si la memoria de los payloads estan alineadas
