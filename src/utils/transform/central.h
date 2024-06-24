#ifndef CENTRAL_H
#define CENTRAL_H

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
#define TO_TRANSFORM "../auxM/"
#define SUCCESS "254"
#define FAILURE "255"
#define MAX_BUFFER_SIZE 1049
#define TKN "-"
#define MAX_SLAVES 5
#define SLAVE_NAME "./slave.exe"

typedef struct{
    int pid;                    //Process pid
    int toSlavePipe[2];         //Pipes to send and receive messages to the slave
    int fromSlavePipe[2];
} SlaveInfo;

/**
 * \brief                       Create and return a slave to apply given transformation command.
 * 
 * \param[in] command           Transformation command to apply.
 * 
 * \return                      SlaveInfo struct on success, exit on failure.
 */
SlaveInfo create_slave(char* command);

/**
 * \brief                       Create all slaves to start working on the server and distributes the messages to them.
 * 
 * \param[in] inital_input      First file to be transformed.
 * \param[in] command           Transformation command to apply.
 * 
 * \return                      void
 */
void distribute_tasks( char* initial_input,char* command);

#endif