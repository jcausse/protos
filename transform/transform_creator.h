#ifndef TRANSFORM_CREATOR_H
#define TRANSFORM_CREATOR_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#define MAX_BUFFER_SIZE 1049



#define MAX 25

typedef struct{
    int pid; //Proceso que se ocupa de la transofrmacion
    int toSlavePipe[2];//Pipes correspondientes
    int fromSlavePipe[2];
} SlaveInfo;

SlaveInfo transform(char * input,char* command);

#endif // TRANSFORM_CREATOR_H