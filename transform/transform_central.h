#ifndef TRANSFORM_CENTRAL_H
#define TRANSFORM_CENTRAL_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>

#define MAX_MAILS 100
#define TKN "-"
#define MAX_SLAVES 25
#define NO_ACTIVE_SLAVE -2
#define TO_TRANSFORM "../auxM"
#define INBOX "../inbox"

#define SUCCESS "254"
#define FAILURE "255"

#define MAX_BUFFER_SIZE 1049

typedef struct{
    char* buffer;
}Message;

void tControl(char* command);

Message queue_message(char* buffer);

static void check_dir(const char * dir){
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }
}

typedef struct{
    int pid; //Proceso que se ocupa de la transofrmacion
    int toSlavePipe[2];//Pipes correspondientes
    int fromSlavePipe[2];
} SlaveInfo;

SlaveInfo transform(char* command);

#endif