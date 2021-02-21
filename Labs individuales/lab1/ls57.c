#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "ls57.h"

/*
 * getsize() devuelve (usando fstatat) el tamaño de un archivo
 * que se ubica en el directorio asociado al file descriptor
 * "atdirfd". Si el archivo no es un archivo regular, se devuelve
 * -1.
 *
 * Precondiciones: atdirfd >= 0, filename != NULL.
 */
ssize_t getsize(int atdirfd, const char *filename){
  struct stat st;
  fstatat(atdirfd,filename,&st,AT_SYMLINK_NOFOLLOW);
  if((st.st_mode & __S_IFMT) == __S_IFREG){
    return st.st_size;
  }else{
    return -1;
  }
}


/*
 * lsdir() muestra los contenidos de un directorio, incluyendo
 * los tamaños de archivos regulares (usando getsize).
 *
 * Precondiciones: dir != NULL.
 */
void lsdir(DIR *dir){
  int fd = 0;
  ssize_t bytes = 0;
  struct dirent* files;
  errno = 0;
  files = readdir(dir);

  while(files != NULL){
    fd = dirfd(dir);
    bytes = getsize(fd,files->d_name);
    if(files->d_name[0] != '.'){
      if((bytes != -1)){
        printf("%s\t%zd\n",files->d_name,bytes);
      }else{
        printf("%s\n",files->d_name);
      }
    }
    errno = 0;
    files = readdir(dir);
  }
}

int main(int argc, char* argv[]){

  if(argc > 2){
    printf("Solo se puede ejecutar con 1 o 2 parametros \n");
  }

  DIR* dir;


  if(argc == 1){
    dir = opendir(".");
  }else{
    dir = opendir(argv[1]);
  }

  if (dir == NULL){
    printf("Error al abrir el directorio \n");
    return -1;
  }

  if(errno == ENOTDIR){
    printf("Solo se pueden listar directorios\n");
    return -1;
  }

  lsdir(dir);

  closedir(dir);
  return 0;
}
