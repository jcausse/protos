#include "transform_creator.h"

SlaveInfo transform(char * input,char* command){    

    SlaveInfo slave;
    
    if (pipe(slave.toSlavePipe) == -1 || pipe(slave.fromSlavePipe) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
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

    char *command = argv[1];
    char *input = argv[2];

    SlaveInfo slave = transform(input, command);

    printf("Slave PID: %d\n", slave.pid);

    ssize_t nbytes;
    char inputBuffer[MAX_BUFFER_SIZE];  
    char outputBuffer[MAX_BUFFER_SIZE];

    // Process the first input file passed as a parameter
    write(slave.toSlavePipe[1], input, strlen(input) + 1);

    // Read the response from the slave
    nbytes = read(slave.fromSlavePipe[0], outputBuffer, sizeof(outputBuffer) - 1);
    if (nbytes > 0) {
        outputBuffer[nbytes] = '\0';  // Null-terminate the output buffer
        printf("Slave output: %s", outputBuffer);

        // Check if the slave has finished its task
        if (strcmp(outputBuffer, SUCCESS) == 0) {
            printf("Slave completed the task successfully.\n");
        } else {
            printf("Slave failed to complete the task.\n");
        }
    } else {
        perror("read from slave");
    }

    // Process subsequent input from stdin
    while ((nbytes = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer) - 1)) > 0) {
        if (nbytes > MAX_BUFFER_SIZE) {
            fprintf(stderr, "Input too long\n");
            continue;
        }

        inputBuffer[nbytes] = '\0';  // Null-terminate the input buffer

        write(slave.toSlavePipe[1], inputBuffer, nbytes);

        // Read the response from the slave
        nbytes = read(slave.fromSlavePipe[0], outputBuffer, sizeof(outputBuffer) - 1);
        if (nbytes > 0) {
            outputBuffer[nbytes] = '\0';  // Null-terminate the output buffer
            printf("Slave output: %s", outputBuffer);

            // Check if the slave has finished its task
            if (strcmp(outputBuffer, SUCCESS) == 0) {
                printf("Slave completed the task successfully.\n");
            } else {
                printf("Slave failed to complete the task.\n");
            }
        } else {
            perror("read from slave");
        }
    }

    close(slave.toSlavePipe[1]);
    close(slave.fromSlavePipe[0]);

    // Wait for the slave process to finish
    int status;
    waitpid(slave.pid, &status, 0);

    return 0;
}
