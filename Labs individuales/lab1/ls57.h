#include <dirent.h>
#include <sys/types.h>

/*
 * getsize() devuelve (usando fstatat) el tamaño de un archivo
 * que se ubica en el directorio asociado al file descriptor
 * "atdirfd". Si el archivo no es un archivo regular, se devuelve
 * -1.
 *
 * Precondiciones: atdirfd >= 0, filename != NULL.
 */
ssize_t getsize(int atdirfd, const char *filename);


/*
 * lsdir() muestra los contenidos de un directorio, incluyendo
 * los tamaños de archivos regulares (usando getsize).
 *
 * Precondiciones: dir != NULL.
 */
void lsdir(DIR *dir);
