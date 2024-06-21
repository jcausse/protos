#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX 25

typedef struct {
    int pid; //Proceso que se ocupa de la transofrmacion
    int toSlavePipe[2];//Pipes correspondientes
    int fromSlavePipe[2];
} SlaveInfo;

static void check_dir(char * dir){
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir);
    }
}

SlaveInfo transform(char * input,const char* command);