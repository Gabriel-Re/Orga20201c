#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "cp57.h"


size_t writeall(int fd, const void *buf, size_t count){
  ssize_t bytes_written;
  size_t pos = 0;
  do{
    bytes_written = write(fd,(buf + pos),count);
    if(bytes_written != -1){
      pos += bytes_written;
    }else if ((bytes_written == -1) && (errno != EINTR)){
      return pos;
    }
  }while((pos < count) && (bytes_written < 0));

  return 0;
}


size_t copyfd(int fdin, int fdout, void *buf, size_t size){
  errno = 0;
  ssize_t bytes_read = read(fdin,buf,size);
  size_t count = 0;
  ssize_t bytes_written = 0;

  do{
    if(errno != EINTR){
      bytes_written = writeall(fdout,buf,bytes_read);
      count += bytes_written;
    }else if ((errno != 0) && (errno != EINTR)){
      return count;
    }
    bytes_read = read(fdin,buf,size);
  }while(bytes_read != 0);

  return 0;
}

int main(int argc, char* argv[]){
  uint8_t buf[512];
  ssize_t reader;
  int origin = 0;
  ssize_t link_reader = 0;
  if(argc != 3){
    printf("No escribiste 3 argumentos, escribiste %i argumentos \n",argc);
    return 0;
  }

  link_reader = readlink(argv[1],(char *)buf,512);
  if(link_reader > -1){
    int symbolic_link = 0;
    symbolic_link = link(argv[1],argv[2]);
    return symbolic_link;
  }
  errno = 0;
  origin = open(argv[1],O_RDONLY);

  if(origin == -1 || errno == EISDIR){
    fprintf(stderr,"Error al abrir '%s': %m \n",argv[1]);
    return -1;
  }

  errno = 0;
  int destination = open(argv[2],O_WRONLY | O_CREAT | O_TRUNC,0666);
  if((errno != 0) && (errno != EISDIR)){
    fprintf(stderr,"Error al abrir '%s': %m \n",argv[2]);
    close(origin);
    return -1;
  }

  if(errno == EISDIR){
    errno = 0;
    int destination_dir = open(argv[2],O_RDONLY); // == dirfd
    if(destination_dir == -1){
      fprintf(stderr,"Error al abrir '%s': %m \n",argv[2]);
      close(origin);
      close(destination);
      return -1;
    }
    close(destination);
    char* dest;
    dest = strrchr(argv[1],'/');
    if(dest != NULL){
      dest ++;
    }else{
      dest = argv[1];
    }
    destination = openat(destination_dir,dest,O_WRONLY | O_CREAT | O_TRUNC,0666);
    close(destination_dir);
    if(destination != -1 ){
      reader = copyfd(origin,destination,buf,sizeof(buf));
    }
  }else{
    reader = copyfd(origin,destination,buf,sizeof(buf));
  }

  if(reader == 0){
    printf("Se copió todo el archivo\n");
  }else{
    printf("Hubo un error o llegaste a copiar .... %zu bytes \n",reader);
  }

  close(origin);
  close(destination);
  return 0;
}
