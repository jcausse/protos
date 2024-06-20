#include "transformer_creator.h"

SlaveInfo transform(char * input,const char* command){    

    SlaveInfo slave;
    
    pipe(slave.toSlavePipe);
    pipe(slave.fromSlavePipe);
    
    if ((slave.pid = fork()) == 0) {
            // Código para el proceso hijo (slave)
        close(slave.toSlavePipe[1]);
        close(slave.fromSlavePipe[0]);

            // Redirecciona las entradas/salidas estándar según sea necesario
        dup2(slave.toSlavePipe[0], STDIN_FILENO);
        dup2(slave.fromSlavePipe[1], STDOUT_FILENO);

        execve(command, input, NULL);

        perror("execve");
        exit(EXIT_FAILURE);
        } else if (slave.pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
        } 
        return slave;
    }
