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

#define INBOX "../inbox/"
#define SUCCESS "254"
#define FAILURE "255"
#define MAX_BUFFER_SIZE 1049

#define MAX_MAILS 100
#define TKN "-"
#define MAX_SLAVES 5
#define NO_ACTIVE_SLAVE -2
#define TO_TRANSFORM "../auxM/"

typedef struct{
    char* buffer;
}Message;

/*void tControl(char* command);

Message queue_message(char* buffer);*/

typedef struct{
    int pid; //Proceso que se ocupa de la transofrmacion
    int toSlavePipe[2];//Pipes correspondientes
    int fromSlavePipe[2];
} SlaveInfo;

SlaveInfo create_slave(char* command);

#endif