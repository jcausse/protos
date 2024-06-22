#include "transform_central.h"

SlaveInfo transform(char* command){    

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
        char * args[] = {"slave.exe",command,NULL};
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

void distribute_tasks(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <command> <input>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *command = argv[1];
    char *initial_input = argv[2];

    check_dir(INBOX);

    SlaveInfo slaves[MAX_SLAVES];
    for (int i = 0; i < MAX_SLAVES; i++) {
        slaves[i] = create_slave(command);
        printf("Slave %d PID: %d\n", i, slaves[i].pid);
    }

    int current_slave = 0;

    // Process the first input file passed as a parameter
    write(slaves[current_slave].toSlavePipe[1], initial_input, strlen(initial_input) + 1);

    char outputBuffer[MAX_BUFFER_SIZE];
    ssize_t nbytes;

    // Read the response from the current slave
    nbytes = read(slaves[current_slave].fromSlavePipe[0], outputBuffer, sizeof(outputBuffer) - 1);
    if (nbytes > 0) {
        outputBuffer[nbytes] = '\0';  // Null-terminate the output buffer
        printf("Slave %d output: %s", current_slave, outputBuffer);

        // Check if the slave has finished its task
        if (strcmp(outputBuffer, SUCCESS) == 0) {
            printf("Slave %d completed the task successfully.\n", current_slave);
        } else {
            printf("Slave %d failed to complete the task.\n", current_slave);
        }
    } else {
        perror("read from slave");
    }

    current_slave = (current_slave + 1) % MAX_SLAVES;

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
            printf("Slave %d output: %s", current_slave, outputBuffer);

            // Check if the slave has finished its task
            if (strcmp(outputBuffer, SUCCESS) == 0) {
                printf("Slave %d completed the task successfully.\n", current_slave);
            } else {
                printf("Slave %d failed to complete the task.\n", current_slave);
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
}

int main(int argc, char* argv[]) {
    distribute_tasks(argc, argv);
    return 0;
}
