#include "transform_creator.h"

SlaveInfo transform(char * input,char* command){    

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
        char * args[] = {"slave.exe",command,input,NULL};
        execve("./slave.exe",args, NULL);

        perror("execve");
        exit(EXIT_FAILURE);
        } else if (slave.pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
        }else {
            // Código para el proceso padre
            close(slave.fromSlavePipe[1]);
            close(slave.toSlavePipe[0]);
        } 
        return slave;
    }

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <command> <input>\n", argv[0]);
        return 1;
    }

    char *input = argv[1];
    char *cmd = argv[2];
    SlaveInfo slave = transform(input, cmd);

    printf("Slave PID: %d\n", slave.pid);

    ssize_t nbytes;
    char inputBuffer[MAX_BUFFER_SIZE];
    char outputBuffer[MAX_BUFFER_SIZE];

    while ((nbytes = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer) - 1)) > 0) {
        if (nbytes > MAX_BUFFER_SIZE) {
            fprintf(stderr, "Input too long\n");
            continue;
        }

        inputBuffer[nbytes] = '\0';  // Null-terminate the input buffer

        write(slave.toSlavePipe[1], inputBuffer, nbytes);

        // Read the output from the slave
        nbytes = read(slave.fromSlavePipe[0], outputBuffer, sizeof(outputBuffer) - 1);
        if (nbytes > 0) {
            outputBuffer[nbytes] = '\0';  // Null-terminate the output buffer
            write(STDOUT_FILENO, outputBuffer, nbytes);
        } else {
            perror("read from slave");
        }
    }

    close(slave.toSlavePipe[1]);
    close(slave.fromSlavePipe[0]);

    int status;
    waitpid(slave.pid, &status, 0);

    return 0;
}
