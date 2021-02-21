#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"



static const int WSIZE = 8;
static const int DSIZE = 16;
#define CHUNKSIZE (1<<12)        /*Extiende el heap 4096 bytes */

// Por favor completar.
team_t team = {
    "g2",  // Número de grupo,
    "Tomas Ayala",     // Nombre integrante 1
    "tayala@fi.uba.ar",     // E-mail integrante 1
    "Gabriel Re",     // Nombre integrante 2 (si lo hay)
    "gre@fi.uba.ar"      // E-mail integrante 2 (si lo hay)
};

void print_block(char* block);

//Puntero al footer del prologo
static char* heap_listp;

// Devuelve el elemento mas grande
static uint64_t max(uint64_t x,uint64_t y){
  return ((x) > (y)? (x) : (y));
}

// Junta el tamaño/contenido con el bit de alocamiento
static uint64_t pack_word(uint64_t size,uint64_t alloc){
  return ((size) | (alloc));
}

// Obtiene el word (8 bytes) que contiene la informacion del header
static uint64_t obtener_word(void* header){
  return (*(uint64_t *)(header));
}

// Cambia el header de un bloque de memoria
static void escribir_header(void* header, uint64_t val){
  return (*(uint64_t *)(header) = (val));
}

//Obtiene el tamaño de un bloque
static uint64_t block_size(void* p){
  return (obtener_word(p) & ~0xF);
}

// Devuelve un bool diciendo si un bloque esta asignado o no
static bool is_allocated(void* p){
  return (obtener_word(p) & 0x1);
}

// Devuelve un puntero al header de un bloque actual
static char* obtener_header(void* block){
  return ((char *)(block) - WSIZE);
}

// Devuelve un puntero al footer del bloque
static char* obtener_footer(void* block){
  return ((char *)(block) + block_size(obtener_header(block)) - DSIZE);
}

// Devuelve un puntero al proximo bloque
static char* proximo_bloque(void* block){
  return ((char *)(block) + block_size(((char *)(block) - WSIZE)));
}

// Devuelve un puntero al bloque anterior
static char* bloque_anterior (void *block){
  return ((char *)(block) - block_size(((char *)(block) - DSIZE)));
}

void check_heap( char* line);


static void* coalesce(void* block){
  bool prev_alloc = is_allocated( obtener_footer(bloque_anterior(block)));
  bool next_alloc = is_allocated( obtener_header( proximo_bloque(block)));
  uint64_t new_size = block_size(obtener_header(block));
  if(prev_alloc && next_alloc)
    return block;
  else if( prev_alloc && !next_alloc){
    new_size += block_size( obtener_header(proximo_bloque(block)));
    escribir_header( obtener_header(block), pack_word(new_size,0));
    escribir_header( obtener_footer(block), pack_word(new_size,0));
  }
  else if( !prev_alloc && next_alloc){
    new_size += block_size(obtener_header(bloque_anterior(block)));
    escribir_header( obtener_footer(block), pack_word(new_size , 0));
    escribir_header( obtener_header(bloque_anterior(block)), pack_word(new_size, 0));
    block = bloque_anterior(block);
  }
  else{
    new_size += block_size(obtener_header(bloque_anterior(block))) + block_size(obtener_footer(proximo_bloque(block)));
    escribir_header( obtener_header(bloque_anterior(block)), pack_word(new_size, 0) );
    escribir_header( obtener_footer(proximo_bloque(block)), pack_word(new_size, 0));
    block = bloque_anterior(block);
  }
  return block;
}

static void* extend_heap(uint64_t words){
  char* block_pointer;
  uint64_t size;

  // Asigna un numero par de palabras para mantener el offset
  size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
  if( ((long) (block_pointer = mem_sbrk(size))) == (void*)-1)
    return NULL;

  // Inicializa un bloque libre con su header y footer y el bloque de epilogo
  escribir_header( obtener_header(block_pointer), pack_word(size, 0));   //Header del nuevo bloque libre
  escribir_header( obtener_footer(block_pointer), pack_word(size,0));    //Footer del nuevo bloque libre
  uint64_t pack_header_epilogo = pack_word(0, 1);
  escribir_header( obtener_header( proximo_bloque(block_pointer)), pack_header_epilogo);   // Nuevo header del epilogo
  check_heap(block_pointer);

  // Une los bloque si antes habia un bloque libre
  block_pointer = coalesce(block_pointer);
  check_heap(block_pointer);
  return block_pointer;
}

/*
 * mm_init - initialize the malloc pack_wordage.
 */
int mm_init(void) {
  if( (heap_listp = mem_sbrk(4*WSIZE)) == (void*) -1)
    return 0;
  escribir_header(heap_listp, 0);
  uint64_t pack_header_prologo = pack_word(DSIZE, 1);
  escribir_header(heap_listp + WSIZE, pack_header_prologo);
  uint64_t pack_footer_prologo = pack_word(DSIZE, 1);
  escribir_header(heap_listp + (2*WSIZE), pack_footer_prologo); //Te queda apuntando aca
  uint64_t pack_header_epilogo = pack_word(0, 1);
  escribir_header(heap_listp + (3*WSIZE), pack_header_epilogo);
  heap_listp+= (2*WSIZE);

  if( extend_heap( CHUNKSIZE/ WSIZE) == NULL)
    return 0;
  return 1;
}

/*
* Primer paso: Conseguir cuanto te va a quedar pesansdo cada bloque, si el bloque pesa lo mismo hacmeos lo que ya tenemos
* Segundo paso: En caso de que el tamanio pedido no sea igual al tamaño del bloque, hay que partir un bloque en dos:
      Para eso: -Tenemos que modificar el tamanio del header actual, (y del footer actual tambien ?)
                - Moves el puntero del bloque el nuevo tamanio y despues de eso creas un nuevo footer, lo moves una palabra y creas el nuevo header?
*/
void place( char* block_pointer, uint64_t size){
  uint64_t size_header = block_size(obtener_header(block_pointer));
  if( (size_header - size) >=  DSIZE *2  ){
    escribir_header( obtener_header(block_pointer), pack_word(size + 0,1));
    escribir_header( obtener_footer(block_pointer), pack_word(size + 0, 1));
    block_pointer = proximo_bloque(block_pointer);
    escribir_header( obtener_header(block_pointer), pack_word(size_header - size - 0 , 0));
    escribir_header( obtener_footer(block_pointer), pack_word(size_header - size - 0 , 0));
  }else{
    escribir_header( obtener_header(block_pointer), pack_word(size_header, 1));
    escribir_header( obtener_footer(block_pointer), pack_word(size_header, 1));
  }
}


static void* find_fit(uint64_t size){
  bool found = false;
  void* block_found = heap_listp ;
  uint64_t size_block = block_size(obtener_header(block_found));
  while( size_block > 0 && !found){
    check_heap(block_found);
    if( !is_allocated(obtener_header(block_found)) && block_size(obtener_header(block_found)) >= size)
      found = true;
    else{
      block_found = proximo_bloque(block_found);
      size_block = block_size(obtener_header(block_found));
    }
  }
  if(!found)
    return NULL;
  return block_found;
}


/*
 * mm_malloc - Allocate a block.
 */
void *mm_malloc(uint64_t size) {
  if(size == 0)
    return NULL;

  if(heap_listp == NULL){
  	mm_init();
  }

  uint64_t adjusted_size = 0;             //asize
  uint64_t extendsize = 0;
  char* block_pointer;

  // Ajusta el tamanio solicitado para incluir los requsitos de aliamiento y los encabezados
  if(size <= DSIZE)
    adjusted_size = 2*DSIZE;
  else
    adjusted_size = DSIZE * (((size + DSIZE) + (DSIZE-1)) / DSIZE);

  // Busca el primer bloque libre del tamanio
  block_pointer = find_fit(adjusted_size);
  if( block_pointer != NULL){
    check_heap( block_pointer);
    place( block_pointer, adjusted_size);
    return block_pointer;
  }

  // Sino se encuentrra el fit. Pedis mas memoria
  extendsize = max(adjusted_size, CHUNKSIZE);
  block_pointer = extend_heap( extendsize/WSIZE);
  if( block_pointer == NULL)
    return NULL;
  place(block_pointer, adjusted_size);
  check_heap(block_pointer);
  return block_pointer;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
  if(ptr == NULL){
    return;
  }

  if(heap_listp == NULL){
    mm_init();
  }

  check_heap(ptr);

  uint64_t size = block_size( obtener_header(ptr));

  escribir_header( obtener_header(ptr), pack_word(size,0));
  escribir_header( obtener_footer(ptr), pack_word(size,0));

  coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, uint64_t size) {
  if(size == 0){
    mm_free(ptr);
    return NULL;
  }
  if(!ptr){
    return mm_malloc(size);
  }
  check_heap(ptr);
  void* newblock = mm_malloc(size);
  if(!newblock){
    return NULL;
  }
  check_heap( newblock);
  uint64_t oldsize = block_size(obtener_header(ptr));
  if( oldsize > size)
    oldsize = size;
  memcpy(newblock,ptr,oldsize);
  mm_free(ptr);
  return newblock;
}

void print_line(char* line, bool* printed){
  printf("Bloque %x\n",  line);
  *printed = true;
}

void check_heap( char* line){
  bool printeo_block = false;
  if( (uint64_t) line % DSIZE != 0){
    if( !printeo_block)
      print_line(line, &printeo_block);
    printf("El bloque esta alineado incorrectament por %i\n", (uint64_t) line % DSIZE );
  }
  if( block_size(obtener_header(line)) != block_size(obtener_footer(line)) ){
    if( !printeo_block)
      print_line(line, &printeo_block);
    printf("Tamaño header %li\n",block_size(obtener_header(line)));
    printf("Tamaño footer %li\n",block_size(obtener_footer(line)));
  }
  if( is_allocated(obtener_header(line)) != (bool)(obtener_word((obtener_footer(line))) & 0x1)){
    if( !printeo_block)
      print_line(line, &printeo_block);
    printf("Bit de asignamiento del header %i \n", is_allocated(obtener_header(line)));
    printf("Bit de asignamiento del footer %i \n", (bool)(obtener_word((obtener_footer(line))) & 0x1));
  }
}
