#include "central.h"
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


SlaveInfo create_slave(char* command){    

    SlaveInfo slave;
    
    if (pipe(slave.toSlavePipe) == -1 || pipe(slave.fromSlavePipe) == -1) {
        perror("pipe");
        slave.pid = -1;
        return slave;
    }
    
    if ((slave.pid = fork()) == 0) {
            // Código para el proceso hijo (slave)
        close(slave.toSlavePipe[1]);
        close(slave.fromSlavePipe[0]);

            // Redirecciona las entradas/salidas estándar según sea necesario
        dup2(slave.toSlavePipe[0], STDIN_FILENO);
        dup2(slave.fromSlavePipe[1], STDOUT_FILENO);
        char * args[] = {SLAVE_NAME,command,NULL};
        execve(SLAVE_NAME,args, NULL);

        perror("execve");
        slave.pid = -1;
        return slave;

        } else if (slave.pid == -1) {
        perror("fork");
        slave.pid = -1;
        return slave;

        }else {
            // Código para el proceso padre
            close(slave.fromSlavePipe[1]);
            close(slave.toSlavePipe[0]);
        } 
        return slave;
    }

int main( int argc, char** argv){

    char *command = argv[1];

    SlaveInfo slaves[MAX_SLAVES];
    for (int i = 0; i < MAX_SLAVES; i++) {
        slaves[i] = create_slave(command);
        if(slaves[i].pid == -1){
            perror("create_slave");
            return 0;
        }
    }

    int current_slave = 0;
    char outputBuffer[MAX_BUFFER_SIZE];
    ssize_t nbytes;

    char inputBuffer[MAX_BUFFER_SIZE];
    // Process subsequent input from stdin
    while ((nbytes = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer) - 1)) > 0) {
        if (nbytes > MAX_BUFFER_SIZE) {
            fprintf(stderr, "Input too long\n");
            continue;
        }

        inputBuffer[nbytes] = '\0';  // Null-terminate the input buffer

        write(slaves[current_slave].toSlavePipe[1], inputBuffer, nbytes);

        // Read the response from the current slave
        nbytes = read(slaves[current_slave].fromSlavePipe[0], outputBuffer, sizeof(outputBuffer) - 1);
        if (nbytes > 0) {
            outputBuffer[nbytes] = '\0';  // Null-terminate the output buffer

            // Check if the slave has finished its task
            if (strcmp(outputBuffer, SUCCESS) == 0) {
               write(STDOUT_FILENO, SUCCESS, strlen(SUCCESS) + 1);
            } else {
                write(STDOUT_FILENO,FAILURE,strlen(FAILURE)+1);
            }
        } else {
            perror("read from slave");
        }

        current_slave = (current_slave + 1) % MAX_SLAVES;
    }

    // Close pipes and wait for all slave processes to finish
    for (int i = 0; i < MAX_SLAVES; i++) {
        close(slaves[i].toSlavePipe[1]);
        close(slaves[i].fromSlavePipe[0]);
        int status;
        waitpid(slaves[i].pid, &status, 0);
    }
    return 1;
}
