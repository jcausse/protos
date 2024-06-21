#include "transform_central.h"


static Message queue_message(char* buffer){
    Message mail;
    strcpy(mail.buffer,buffer);
    return mail;    
}

static int check_for_slaves(int*status){
    for(int i = 0; i < MAX; i++){
        if(status[i] == 0){
            return i;
        }
    }
    return NO_ACTIVE_SLAVE;
}

static void readTransformations(Message* queue, int queue_size, SlaveInfo* slaves, int* slave_status, fd_set readFromSlaves, int maxFd, int* active){
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
        kill(slaves[i].pid,SIGTERM);
    }
    free(slaves);
}

void tControl(const char* command){
    check_dir(INBOX);

    int active_slaves = 0;
    int slave_status[MAX] = {0}; //Si esta en 0 el slave esta libre para poder ser utilizado
    SlaveInfo slaves[MAX] = calloc(MAX,sizeof(SlaveInfo));
    int maxFd = 0;
    fd_set readFromSlaves;
    FD_ZERO(&readFromSlaves);
    ssize_t nbytes;
    char input[1024];
    int queue_size = 0; //cantidad mails para transformar
    Message queue[MAX_MAILS]; //cola de mails a transformar
    int go_ahead = NO_ACTIVE_SLAVE;
    char* cID;
    int active = 1;
    char aux[1024];
    
    while(active == 1){
        /*YOU GOT MAIL!*/
        if((nbytes = read(STDIN_FILENO, input, sizeof(input))) > 0){
        /*si me pasen mails para transformar y no tengo en la queue ninguno*/
            strcpy(aux,input);
            cID = strtok(aux,TKN);
            check_dir(cID);

            if(queue_size == 0){

                /*Si no hay slaves, creo uno y lo pongo a trabajar*/
                if(active_slaves == 0){
                    go_ahead = NO_ACTIVE_SLAVE;
                }else{
                    go_ahead = check_for_slaves(slave_status);
                }
                
                /*Si no hay nadie libre o aun no cree ningun slave lo meto en la queue */
                if(go_ahead == NO_ACTIVE_SLAVE && queue_size < MAX_MAILS){
                queue[queue_size] = queue_message(input);
                queue_size++;
                }else{
                    write(slaves[go_ahead].toSlavePipe[1], input, strlen(input) + 1);
                }
                
                /* Si no hay slaves libres, aun no tengo el maximo adimitido, creo uno y lo  pongo a trabajar*/
                if(active_slaves < MAX && queue_size > 0){
                    slaves[active_slaves] = transform(input,command);
                    
                    close(slaves[active_slaves].fromSlavePipe[1]);
                    close(slaves[active_slaves].toSlavePipe[0]);

                    FD_SET(slaves[active_slaves].fromSlavePipe[0], &readFromSlaves);
                    if (slaves[active_slaves].fromSlavePipe[0] > maxFd) {
                        maxFd = slaves[active_slaves].fromSlavePipe[0];
                    }
                    slave_status[active_slaves] = 1;
                    active_slaves++;    
                }
            }else{/*Si hay mails para procesar antes los mando a la queue*/
                queue[queue_size] = queue_message(input);
                queue_size++;
            }
        }else{
        /*Si no hay mails para procesar, me fijo si hay algun slave que me haya mandado algo*/
            readTransformations(queue,queue_size,slaves,slave_status,readFromSlaves,maxFd,&active);
        }
    }

    while(active_slaves != 0){
        readTransformations(queue,queue_size,slaves,slave_status,readFromSlaves,maxFd,&active);
    }
    free_slaves(slaves);
}
