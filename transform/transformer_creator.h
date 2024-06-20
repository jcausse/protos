#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <winsock.h>
#define MAX 25

typedef struct {
    int pid; //Proceso que se ocupa de la transofrmacion
    int toSlavePipe[2];//Pipes correspondientes
    int fromSlavePipe[2];
} SlaveInfo;

SlaveInfo transform(char * input,const char* command);