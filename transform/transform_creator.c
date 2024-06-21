#include "transform_creator.h"

/*SlaveInfo transform(char * input,char* command){    

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

int main(int argc,char* argv[]){
    if(argc < 3){
        return 1;
    }
    char* input = argv[1];
    char * cmd = argv[2];
    SlaveInfo slave = transform(input,cmd);
    printf("%d \n",slave.pid);
    ssize_t nbytes;
    char inputBuffer[MAX_BUFFER_SIZE];  
    while ((nbytes = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer))) > 0) {
       if (nbytes > MAX_BUFFER_SIZE) {
            fprintf(stderr, "Input too long\n");
            continue;
        }
        if (write(slave.toSlavePipe[1], inputBuffer, nbytes) == -1) {
            perror("write to slave");
            continue;
        }
    }
    close(slave.toSlavePipe[1]);

    return 0;
}*/

