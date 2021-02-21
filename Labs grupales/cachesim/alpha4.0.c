#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define FORMATO_LECTURA_TRAZA "%x: %c %x %i %x\n"
#define ERROR -1
#define ERROR_ARCHIVO -2
#define LECTURA 'R'
#define MISS_PENALTY 100
#define DESALOJO_LIMPIO 1
#define DESALOJO_DIRTY_BIT 2
#define VERBOSO_HIT 1
#define VERBOSO_MISS_CLEAN "2a"
#define VERBOSO_MISS_DIRTY "2b"

typedef struct modo_ver{
  int operacion_actual;
  long num_operacion;
  int set_operado;
  int tag_operado;
  int linea_operada;
  int tag_anterior;
  bool bit_validez;
  bool dirty_bit;
  long ultima_operacion;
  bool mapeo_directo;
}modo_verboso_t;

typedef struct linea{
  int tag;
  int num_linea;
  bool dirty_bit;
  bool valido;
  long ultimo_ciclo_usado;
  int ultima_operacion;
}linea_t;

typedef struct set{
  linea_t* lineas;
  int lineas_por_set;
  int lineas_ocupadas;
}set_t;


typedef struct metricas{
  int lecturas;
  int escrituras;
  long total_accesos;
  int misses_read;
  int misses_write;
  long total_misses;
  int dirty_read_miss;
  int dirty_write_miss;
  long bytes_read;
  long bytes_written;
  long ciclos_lecturas;
  long ciclos_escrituras;
  double miss_rate;
}metricas_t;


typedef struct cache{
  metricas_t metricas;
  set_t* sets;
  int cantidad_sets;
  int tamanio_bloque;
  long tamanio_cache;
  long ciclos;
}cache_t;

typedef struct traza{
  uint32_t instruction_pointer;
  uint32_t direccion_accedida;
  uint32_t datos_leidos;
  char instruccion;
  int cant_de_bytes;
}traza_t;

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
set_t* crear_sets( int asociatividad_cache,int numero_sets_cache){
  set_t* set = calloc(1,sizeof(set_t) * (size_t) numero_sets_cache);
  if(set){
    for(int j = 0; j< numero_sets_cache; j++)
      set[j].lineas_por_set = asociatividad_cache;
    bool funciona_correctamente = true;
    int i = 0;
    while(funciona_correctamente && i < numero_sets_cache){
      set[i].lineas = calloc(1,sizeof(linea_t) * (size_t) asociatividad_cache);
      if(!set[i].lineas)
        funciona_correctamente = false;
      else{
        if(asociatividad_cache > 1){
          for(int j = 0; j < asociatividad_cache; j++)
            set[i].lineas[j].num_linea = j;
        }
        i++;
      }
    }
    while(!funciona_correctamente && i >= 0){
      free(set[i].lineas);
      i--;
    }
    if(!funciona_correctamente){
      free(set);
      set = NULL;
    }
  }
  return set;
}

cache_t* crear_cache(long tamanio_cache, int asociatividad_cache, int numero_sets_cache){
  cache_t* cache = calloc(1, sizeof(cache_t));
  if(cache){
    cache->tamanio_bloque = (int) (tamanio_cache/(asociatividad_cache * numero_sets_cache) );
    cache->tamanio_cache = tamanio_cache;
    cache->cantidad_sets = numero_sets_cache;
    cache->sets = crear_sets( asociatividad_cache, numero_sets_cache);
    if(!cache->sets)
      return NULL;
  }
  return cache;
}

void cache_destruir(cache_t* cache){
  for(int i = 0; i< cache->cantidad_sets; i++){
    free(cache->sets[i].lineas);
  }
  free(cache->sets);
  free(cache);
}

bool es_potencia_de_dos( int numero){
  long potencia = 0;
  int i = 0;
  while( potencia < numero){
    potencia = (long) pow(2, i);
    i++;
  }
  return potencia == numero;
}

bool son_potencias_de_dos( long tamanio_cache, int asociatividad_cache, int numero_sets_cache){
  return es_potencia_de_dos( (int) tamanio_cache) && es_potencia_de_dos(asociatividad_cache) && es_potencia_de_dos(numero_sets_cache);
}

bool combinacion_correcta(int tamanio_cache, int asociatividad_cache, int numero_sets_cache){
  int tamanio_bloque = tamanio_cache / (asociatividad_cache * numero_sets_cache);
  int chequeo_cache = tamanio_bloque * asociatividad_cache * numero_sets_cache;
  return tamanio_cache == chequeo_cache;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------

int leer_set( set_t set, int tag_buscado){
  bool encontrado = false;
  int i = 0;
  while( i< set.lineas_por_set && !encontrado){
    if( set.lineas[i].valido && set.lineas[i].tag == tag_buscado)
      encontrado = true;
    else
      i++;
  }
  return encontrado ? i : ERROR;
}

void calcular_set_y_tag( cache_t* cache, uint32_t direccion_accedida, int bits_offset, int* set, int* tag){

  int bits_set = (int) log2((double)cache->cantidad_sets);
  bits_offset = (int) log2((double)bits_offset);
  int bits_tag = 32 - (bits_set + bits_offset);
  uint32_t mascara_tag = (uint32_t) ((1 <<31) >> (bits_tag-1));
  uint32_t mascara_offset= (uint32_t) (~ ( ((1 <<31) ^ (~0)) << bits_offset) );
  uint32_t mascara_set = (~(mascara_tag | mascara_offset)) >> bits_offset ;
  *set = (int32_t) ((direccion_accedida >> (bits_offset)) & mascara_set);
  *tag = (int32_t) ((direccion_accedida) >> (bits_set + bits_offset) );
}

void actualizar_ciclos_lectura( set_t* set, int tag, long ciclos){
  int i = 0;
  bool escribio = false;
  while( i < set->lineas_por_set && !escribio){
    if( set->lineas[i].tag == tag){
      set->lineas[i].ultimo_ciclo_usado = ciclos + 1;
      set->lineas[i].valido = true;
      escribio = true;
    }else{
      i++;
    }
  }
}

void imprimir_hit( modo_verboso_t* verboso, int set, int tag, int num_linea, linea_t linea_actual){
  if(!verboso->mapeo_directo)
    printf("%i %i %x %x %i %x %i %i %i\n",verboso->operacion_actual, VERBOSO_HIT, set, tag, num_linea, tag, 1, linea_actual.dirty_bit, linea_actual.ultima_operacion);
  else
    printf("%i %i %x %x %i %x %i %i\n",verboso->operacion_actual, VERBOSO_HIT, set, tag, num_linea, tag, 1, linea_actual.dirty_bit);
}

bool leer_cache(cache_t* cache,uint32_t direccion_accedida, modo_verboso_t* verboso){
  int linea_buscada;
  int tag, set;
  calcular_set_y_tag( cache, direccion_accedida, cache->tamanio_bloque, &set, &tag);
  linea_buscada = leer_set( cache->sets[set], tag);
  if( linea_buscada != ERROR){
    if( verboso){
      imprimir_hit(verboso, set, tag, linea_buscada, cache->sets[set].lineas[linea_buscada]);
      cache->sets[set].lineas[linea_buscada].ultima_operacion = verboso->operacion_actual;
    }
    actualizar_ciclos_lectura( &cache->sets[set], tag, cache->ciclos);
  }
  return linea_buscada != ERROR;
}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------

linea_t* linea_no_valida( set_t* set){
  bool encontrado = false;
  int i = 0;
  while( i < set->lineas_por_set && !encontrado){
    if( !set->lineas[i].valido)
      encontrado = true;
    else
      i++;
  }
  return encontrado ? (set->lineas + i) : NULL;
}

void cargar_linea( linea_t* linea, int tag, long ciclos, char instruccion){
  linea->valido = true;
  linea->tag = tag;
  linea->ultimo_ciclo_usado = ciclos + MISS_PENALTY + 1;
  if( linea->dirty_bit && instruccion != LECTURA)
    linea->ultimo_ciclo_usado += MISS_PENALTY;
  else if( !linea->dirty_bit && instruccion != LECTURA)
    linea->dirty_bit = true;
  else
    linea->dirty_bit = false;
}

void imprimir_miss_desalojo( modo_verboso_t* verboso, int set, int tag, int num_linea, linea_t linea_actual, const char* tipo_miss){
  if(!verboso->mapeo_directo)
    printf("%i %s %x %x %i %x %i %i %i\n",verboso->operacion_actual, tipo_miss, set, tag, num_linea, linea_actual.tag, 1, linea_actual.dirty_bit, linea_actual.ultima_operacion);
  else
  printf("%i %s %x %x %i %x %i %i\n",verboso->operacion_actual, tipo_miss, set, tag, num_linea, linea_actual.tag, 1, linea_actual.dirty_bit);

}

int desalojar_linea( long ciclos, set_t* set, int tag, char instruccion, modo_verboso_t* verboso, int num_set){
  int pos_desalojar = 0;
  int i = 1;
  while( i < set->lineas_por_set){
    if( set->lineas[pos_desalojar].ultimo_ciclo_usado > set->lineas[i].ultimo_ciclo_usado)
      pos_desalojar = i;
    i++;
  }
  if( set->lineas[pos_desalojar].dirty_bit){
    if(verboso){
      imprimir_miss_desalojo(verboso, num_set, tag, pos_desalojar, set->lineas[pos_desalojar], VERBOSO_MISS_DIRTY);
      set->lineas[pos_desalojar].ultima_operacion = verboso->operacion_actual;
    }
    cargar_linea( &set->lineas[pos_desalojar], tag, ciclos, instruccion);
    return DESALOJO_DIRTY_BIT;
  }
  if(verboso){
    imprimir_miss_desalojo(verboso, num_set, tag, pos_desalojar, set->lineas[pos_desalojar], VERBOSO_MISS_CLEAN);
    set->lineas[pos_desalojar].ultima_operacion = verboso->operacion_actual;
  }
  cargar_linea( &set->lineas[pos_desalojar], tag, ciclos, instruccion);
  return DESALOJO_LIMPIO;
}

void imprimir_miss_limpio( modo_verboso_t* verboso, int set, int tag, int linea){
  if(!verboso->mapeo_directo)
    printf("%i %s %x %x %i %i %i %i %i\n", verboso->operacion_actual, "2a", set, tag, linea, -1, 0, 0, 0);
  else
  printf("%i %s %x %x %i %i %i %i\n", verboso->operacion_actual, "2a", set, tag, linea, -1, 0, 0);

}

int cargar_linea_miss( cache_t* cache, uint32_t direccion_accedida, char instruccion, modo_verboso_t* verboso){
  linea_t* linea_a_desalojar = NULL;
  int tag, set, bits_offset;
  int resultado;
  bits_offset = cache->tamanio_bloque;
  calcular_set_y_tag( cache, direccion_accedida, bits_offset, &set, &tag);
  linea_a_desalojar = linea_no_valida( &cache->sets[set]);
  if( linea_a_desalojar){
    if(verboso){
      imprimir_miss_limpio(verboso, set, tag, cache->sets[set].lineas_ocupadas);
      linea_a_desalojar->ultima_operacion = verboso->operacion_actual;
    }
    cargar_linea( linea_a_desalojar, tag, cache->ciclos, instruccion);
    cache->sets[set].lineas_ocupadas++;
    resultado = DESALOJO_LIMPIO;
  }
  else
    resultado = desalojar_linea( cache->ciclos, &cache->sets[set], tag, instruccion, verboso, set);
  return resultado;
}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------

int actualizar_ciclos_escritura( set_t* set, int tag, long ciclos){
  int i = 0;
  bool escribio = false;
  while( i < set->lineas_por_set && !escribio){
    if( set->lineas[i].tag == tag){
      set->lineas[i].ultimo_ciclo_usado = ciclos + 1;
      set->lineas[i].dirty_bit = true;
      set->lineas[i].valido = true;
      escribio = true;
    }else{
      i++;
    }
  }
  return i;
}


bool escribir_cache( cache_t* cache, uint32_t direccion_accedida, modo_verboso_t* verboso){
  int linea_buscada;
  int tag, set, bits_offset;
  bits_offset = cache->tamanio_bloque;
  calcular_set_y_tag( cache, direccion_accedida, bits_offset, &set, &tag);
  linea_buscada = leer_set( cache->sets[set], tag);
  if( linea_buscada != ERROR){
    if(verboso){
      imprimir_hit(verboso, set, tag, linea_buscada, cache->sets[set].lineas[linea_buscada]);
      cache->sets[set].lineas[linea_buscada].ultima_operacion = verboso->operacion_actual;
    }
    actualizar_ciclos_escritura( &cache->sets[set], tag, cache->ciclos);
  }
  return linea_buscada != ERROR;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------

void actualizar_metricas( metricas_t* metricas, char instruccion, bool fue_hit, int desalojo, int tamanio_bloque){
  if( instruccion == LECTURA){
    metricas->lecturas++;
    if(fue_hit){
      metricas->ciclos_lecturas++;
    }
    else{
      metricas->misses_read++;
      metricas->ciclos_lecturas += MISS_PENALTY + 1;
      metricas->bytes_read += tamanio_bloque;
      if( desalojo == DESALOJO_DIRTY_BIT){
        metricas->dirty_read_miss++;
        metricas->ciclos_lecturas += MISS_PENALTY;
        metricas->bytes_written += tamanio_bloque;
      }
    }
  }else{
    metricas->escrituras++;
    if(fue_hit){
      metricas->ciclos_escrituras++;
    }
    else{
      metricas->bytes_read += tamanio_bloque;
      metricas->misses_write++;
      metricas->ciclos_escrituras += MISS_PENALTY + 1;
      if( desalojo == DESALOJO_DIRTY_BIT){
        metricas->dirty_write_miss++;
        metricas->ciclos_escrituras += MISS_PENALTY;
        metricas->bytes_written += tamanio_bloque;
      }
    }
  }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------

void simular_cache_simple(cache_t* cache, FILE* archivo){
  traza_t traza;
  bool hit;
  int resultado;
  int leidos = fscanf(archivo, FORMATO_LECTURA_TRAZA, &traza.instruction_pointer, &traza.instruccion, &traza.direccion_accedida, &traza.cant_de_bytes, &traza.datos_leidos);
  while( leidos != EOF ){
    if(traza.instruccion == LECTURA)
      hit = leer_cache(cache,traza.direccion_accedida, NULL);
    else
      hit = escribir_cache(cache,traza.direccion_accedida, NULL);
    if(hit == true){
      cache->ciclos++;
      resultado = 0;
    }else{
      resultado = cargar_linea_miss(cache, traza.direccion_accedida, traza.instruccion, NULL);
      cache->ciclos += MISS_PENALTY + 1;
      if(resultado == DESALOJO_DIRTY_BIT){
        cache->ciclos += MISS_PENALTY;
      }
    }
    actualizar_metricas(&cache->metricas, traza.instruccion, hit, resultado, cache->tamanio_bloque);
    leidos = fscanf(archivo, FORMATO_LECTURA_TRAZA, &traza.instruction_pointer, &traza.instruccion, &traza.direccion_accedida, &traza.cant_de_bytes, &traza.datos_leidos);
  }
}



//---------------------------------------------------------------------------------------------------------------------------------------------------------------

void mostrar_metricas( cache_t* cache){
  cache->metricas.total_accesos = cache->metricas.lecturas + cache->metricas.escrituras;
  cache->metricas.total_misses = cache->metricas.misses_read + cache->metricas.misses_write;
  cache->metricas.miss_rate = (double) cache->metricas.total_misses / (double) cache->metricas.total_accesos;
  if( cache->sets->lineas_por_set == 1 )
    printf("direct-mapped, ");
  else
    printf("%i-way, ", cache->sets->lineas_por_set);
  printf("%i sets, size = %liKB\n", cache->cantidad_sets, (cache->tamanio_cache / 1024));
  printf("loads %i stores %i total %li\n",cache->metricas.lecturas,cache->metricas.escrituras,cache->metricas.total_accesos);
  printf("rmiss %i wmiss %i total %li\n",cache->metricas.misses_read,cache->metricas.misses_write,cache->metricas.total_misses);
  printf("dirty rmiss %i dirty wmiss %i\n",cache->metricas.dirty_read_miss,cache->metricas.dirty_write_miss);
  printf("bytes read %li bytes written %li\n",cache->metricas.bytes_read,cache->metricas.bytes_written);
  printf("read time %li write time %li\n",cache->metricas.ciclos_lecturas,cache->metricas.ciclos_escrituras);
  printf("miss rate %lf\n",cache->metricas.miss_rate);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------


bool rango_verboso(int operacion_actual, int primera_operacion, int ultima_operacion){
  return operacion_actual >= primera_operacion && operacion_actual <= ultima_operacion;
}

modo_verboso_t* crear_modo_verboso(int mapeo){
  modo_verboso_t* verboso = calloc(1, sizeof(modo_verboso_t));
  if(verboso)
    verboso->mapeo_directo = mapeo == 1;
  return verboso;
}

void simular_cache_modo_verboso(cache_t* cache, FILE* archivo, int primera_operacion, int ultima_operacion){
  modo_verboso_t* verbos = crear_modo_verboso(cache->sets->lineas_por_set);
  if(!verbos)
    return;
  traza_t traza;
  bool hit;
  int resultado;
  int leidos = fscanf(archivo, FORMATO_LECTURA_TRAZA, &traza.instruction_pointer, &traza.instruccion, &traza.direccion_accedida, &traza.cant_de_bytes, &traza.datos_leidos);
  while( leidos != EOF ){
    if(traza.instruccion == LECTURA)
      hit = leer_cache(cache,traza.direccion_accedida, rango_verboso(verbos->operacion_actual, primera_operacion, ultima_operacion) ? verbos : NULL );
    else
      hit = escribir_cache(cache,traza.direccion_accedida, rango_verboso(verbos->operacion_actual, primera_operacion, ultima_operacion) ? verbos : NULL);
    if(hit == true){
      cache->ciclos++;
      resultado = 0;
    }else{
      resultado = cargar_linea_miss(cache, traza.direccion_accedida, traza.instruccion, rango_verboso(verbos->operacion_actual, primera_operacion, ultima_operacion) ? verbos : NULL);
      cache->ciclos += MISS_PENALTY + 1;
      if(resultado == DESALOJO_DIRTY_BIT){
        cache->ciclos += MISS_PENALTY;
      }
    }
    actualizar_metricas(&cache->metricas, traza.instruccion, hit, resultado, cache->tamanio_bloque);
    verbos->operacion_actual++;
    leidos = fscanf(archivo, FORMATO_LECTURA_TRAZA, &traza.instruction_pointer, &traza.instruccion, &traza.direccion_accedida, &traza.cant_de_bytes, &traza.datos_leidos);
  }
  free( verbos);
}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char const *argv[]) {
  if( argc != 5 && argc != 8)
    return ERROR;
  long tamanio_cache = atol(argv[2]);
  int asociatividad_cache = atoi( argv[3]);
  int numero_sets_cache = atoi( argv[4]);
  if( tamanio_cache <= 0 || !son_potencias_de_dos( tamanio_cache, (long) asociatividad_cache, (long) numero_sets_cache)){

    return ERROR;
  }
  if(!combinacion_correcta( (int) tamanio_cache, asociatividad_cache, numero_sets_cache)){
    perror("Datos invalidos\n");
    return ERROR;
  }
  FILE* archivo = fopen( argv[1], "r");
  if( !archivo){
    printf("No se pudo abrir el archivo\n");
    return ERROR_ARCHIVO;
  }
  cache_t* cache = crear_cache( tamanio_cache, asociatividad_cache, numero_sets_cache);
  if(!cache){
    fclose(archivo);
    return ERROR;
  }
  if(argc == 8){
    int primera_operacion = atoi( argv[6]);
    int ultima_operacion = atoi( argv[7]);
    if( primera_operacion >= ultima_operacion)
      return ERROR;
    simular_cache_modo_verboso(cache, archivo, primera_operacion, ultima_operacion);
  }
  else
    simular_cache_simple( cache, archivo);
  mostrar_metricas(cache);
  fclose(archivo);
  cache_destruir(cache);
  return 0;
}
