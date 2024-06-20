#include "transformer_central.h"


Message queue_message(char* buffer,int cID){
    Message mail;
    mail.buffer = buffer;
    mail.cID = cID;
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

void readTransformations(Message* queue, int queue_size, TransformerInfo* slaves, int* slave_status, fd_set readFromSlaves, int maxFd, int* active){
    fd_set readSet = readFromSlaves;
    int selectResult = select(maxFd + 1, &readSet, NULL, NULL, NULL);

    if (selectResult == -1) {
        perror("select");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < MAX; i++){
            if(FD_ISSET(slaves[i].slave.fromSlavePipe[0],&readSet)){
                char buffer[1024];
                ssize_t bytesRead = read(slaves[i].slave.fromSlavePipe[0], buffer, sizeof(buffer));
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
                }else{                            write(slaves[i].slave.toSlavePipe[1], queue[0].buffer, strlen(queue[0].buffer) + 1);
                        slaves[i].cID = queue[0].cID;
                        queue_size--;
                        for(int j = 0; j < queue_size; j++){
                        queue[j] = queue[j+1];
                    }
                }
            }
        }
    }
}

void transformer(const char* command){
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
    int cID;
    int active = 1;
    
    while(active == 1){
        /*YOU GOT MAIL!*/
        if((nbytes = read(STDIN_FILENO, input, sizeof(input))) > 0){
        /*si me pasen mails para transformar y no tengo en la queue ninguno*/
            cID = atoi(strtok(input,tkn));
            char* mail = strtok(NULL,tkn);
            if(queue_size == 0){

                /*Si no hay slaves, creo uno y lo pongo a trabajar*/
                if(active_slaves == 0){
                    go_ahead = NO_ACTIVE_SLAVE;
                }else{
                    go_ahead = check_for_slaves(slave_status);
                }
                
                /*Si no hay nadie libre o aun no cree ningun slave lo meto en la queue */
                if(go_ahead == NO_ACTIVE_SLAVE && queue_size < MAX_MAILS){
                queue[queue_size] = queue_message(mail,cID);
                queue_size++;
                }else{
                    write(slaves[go_ahead].toSlavePipe[1], mail, strlen(mail) + 1);
                }
                
                /* Si no hay slaves libres, aun no tengo el maximo adimitido, creo uno y lo  pongo a trabajar*/
                if(active_slaves < MAX && queue_size > 0){
                    slaves[active_slaves] = transform(mail,command);
                    
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
                queue[queue_size] = queue_message(mail,cID);
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

}
