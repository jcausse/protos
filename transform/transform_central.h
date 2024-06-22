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
#define TO_TRANSFORM "../auxM/"
#define SUCCESS "254"
#define FAILURE "255"
#define MAX_BUFFER_SIZE 1049
#define MAX_MAILS 100
#define TKN "-"
#define MAX_SLAVES 5

typedef struct{
    int pid; //Proceso que se ocupa de la transofrmacion
    int toSlavePipe[2];//Pipes correspondientes
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

/**
 * \brief                       Checks if the given directory exists, if not creates it.
 * 
 * \param[in] dir               Directory to check.
 *
 * 
 * \return                      void
 */
void check_dir(char * dir);

/**
 * \brief                       Removes a substring from a string. Used to remove the TO_TRANSFORM substring from the user name.
 * 
 * \param[in] string            Given string to clean.
 * \param[in] sub               Substring to remove form the string.
 *
 * 
 * \return                      void
 */
void removeSubstr (char *string, char *sub);

/**
 * \brief                       Creates and runs the command to transform the given mail and leaves it in the inbox of the specified user.
 * 
 * \param[in] mail              Path of the mail to transform.
 * \param[in] command           Transformation command to apply.
 * \param[in] toSave            Path of the inbox to save the transformed mail.
 * 
 * \return                      On success, 0. On failure, -1.
 */
int transform_mail(char * mail, char * command,char * toSave)

#endif