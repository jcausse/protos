#include "transform_central.h"
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


Message queue_message(char* buffer){
    Message mail;
    mail.buffer = malloc(strlen(buffer)+1);
    strcpy(mail.buffer,buffer);
    return mail;    
}

int check_for_slaves(int*status){
    for(int i = 0; i < MAX; i++){
        if(status[i] == 0){
            return i;
        }
    }
    return NO_ACTIVE_SLAVE;
}

/*void readTransformations(Message* queue, int queue_size, SlaveInfo* slaves, int* slave_status, fd_set readFromSlaves, int maxFd, int* active){
    fd_set readSet = readFromSlaves;
    int selectResult = select(maxFd + 1, &readSet, NULL, NULL, NULL);

    if (selectResult == -1) {
        perror("select");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < MAX; i++){
            if(FD_ISSET(slaves[i].fromSlavePipe[0],&readSet)){
                char buffer[1024];
                ssize_t bytesRead = read(slaves[i].fromSlavePipe[0], buffer, sizeof(buffer));
                if (bytesRead == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                } else {
                    buffer[bytesRead] = '\0';
                    size_t output_dim = write(STDOUT_FILENO, buffer, strlen(buffer) + 1);
                    if(output_dim == -1){
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                }
                if(queue_size == 0){
                    slave_status[i] = 0;
                }else{
                    if(strcmp(queue[0].buffer,"-2") != 0){
                        active = 0;
                }else{                            
                        write(slaves[i].toSlavePipe[1], queue[0].buffer, strlen(queue[0].buffer) + 1);
                        queue_size--;
                        for(int j = 0; j < queue_size; j++){
                        queue[j] = queue[j+1];
                    }
                }
            }
        }
    }
}

free_slaves(SlaveInfo* slaves){
    for(int i = 0; i < MAX; i++){
        close(slaves[i].fromSlavePipe[0]);
        close(slaves[i].toSlavePipe[1]);
    }
    free(slaves);
}*/

void tControl(char* command){
    if(command == NULL){
        printf("NO");
        exit(EXIT_FAILURE);
    }
    check_dir(INBOX);

    int active_slaves = 0;
    int slave_status[MAX] = {0}; //Si esta en 0 el slave esta libre para poder ser utilizado
    SlaveInfo slaves[MAX] = {0};
    int maxFd = 0;
    fd_set readFromSlaves;
    FD_ZERO(&readFromSlaves);
    ssize_t nbytes;
    char input[1024];
    char resp[1024];
    int queue_size = 0; //cantidad mails para transformar
    Message queue[MAX_MAILS]; //cola de mails a transformar
    int active = 1;
    
    while(active == 1){ 
        if((nbytes = read(STDIN_FILENO, input, sizeof(input))) > 0){
            input[nbytes -1] = '\0';
            printf("%s",input);
                if(active_slaves == 0){
                    SlaveInfo slave = transform(input,command);
                    close(slaves[active_slaves].fromSlavePipe[1]);
                    close(slaves[active_slaves].toSlavePipe[0]);
                    slaves[active_slaves] = slave;
                    slave_status[active_slaves] = 1;
                }
                /*else if(active_slaves < MAX && queue_size == 0){
                    slaves[active_slaves] = transform(input,command);
                    
                    close(slaves[active_slaves].fromSlavePipe[1]);
                    close(slaves[active_slaves].toSlavePipe[0]);

                    FD_SET(slaves[active_slaves].fromSlavePipe[0], &readFromSlaves);
                    if (slaves[active_slaves].fromSlavePipe[0] > maxFd) {
                        maxFd = slaves[active_slaves].fromSlavePipe[0];
                    }
                    slave_status[active_slaves] = 1;
                    active_slaves++;    
                }*/
                nbytes = read(slaves[0].fromSlavePipe[0],resp,sizeof(resp));
                if (nbytes > 0) {
                    resp[nbytes] = '\0'; 
                    printf("Slave output: %s FROM SLAVE %d", resp,slaves[0].pid);
                    slave_status[0] = 0;
                }
            }
            /*for(int i = 0; i < MAX; i++){
                    if(FD_ISSET(slaves[i].fromSlavePipe[0],&readFromSlaves)){
                        nbytes = read(slaves[0].fromSlavePipe[0],resp,sizeof(resp));
                            if (nbytes > 0) {
                                resp[nbytes] = '\0'; 
                                printf("Slave output: %s", resp);
                                slave_status[0] = 0;
                            }
                        if(queue_size == 0){
                            slave_status[i] = 0;
                        }else{
                            if(strcmp(queue[0].buffer,"-2") != 0){
                                active = 0;
                        }else{                            
                                write(slaves[i].toSlavePipe[1], queue[0].buffer, strlen(queue[0].buffer) + 1);
                                queue_size--;
                                for(int j = 0; j < queue_size; j++){
                                queue[j] = queue[j+1];
                            }
                        }
                    }*/  
            /*else{/
                queue[queue_size] = queue_message(input);
                queue_size++;
            }
        }else{
        
            readTransformations(queue,queue_size,slaves,slave_status,readFromSlaves,maxFd,&active);
        }
    }

   
    } 
    while(active_slaves != 0){
        readTransformations(queue,queue_size,slaves,slave_status,readFromSlaves,maxFd,&active);
    }
    free_slaves(slaves);*/
          
    }
}//end of function

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("ALO");
    }
    tControl(argv[1]);

    /*ssize_t nbytes;
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
    close(slave.toSlavePipe[1]);*/

    return 0;
}