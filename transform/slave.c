#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#define INBOX "../inbox/"
#define SUCCESS "254 \n"
#define FAILURE "255 \n"
#define MAX_BUFFER_SIZE 1049
#define ERR_MSG "Usage: <command> <mail>\n"

static void check_dir(char * dir){
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }
}

int transform_mail(char * mail, char * command,char * toSave) {
    char* com = calloc(300,sizeof(char));
    snprintf(com, 300, "%s %s > %s 2> transform.err", command, mail, toSave);
    printf("COM: %s\n",com);
    int retVal = system(com);
    free(com);
    return retVal;  
}

int main (int argc, char *argv[]) {
    if (argc < 3) {
        printf(ERR_MSG);
    }
    check_dir(INBOX);
    char * command = argv[1];
    char * mail = malloc(strlen(argv[2]) + 1);
    if (mail == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(mail, argv[2]);
    
  
    char * user = strtok(mail, "-");
    if (user == NULL) {
        fprintf(stderr, ERR_MSG);
        free(mail);
        exit(EXIT_FAILURE);
    }

    char * aux = strtok(NULL, "-");
    if (aux == NULL) {
        fprintf(stderr, ERR_MSG);
        free(mail);
        exit(EXIT_FAILURE);
    }

    char * toSave= malloc(strlen(INBOX) + strlen(aux) +strlen(user) + 2);
     if (toSave == NULL) {
        perror("malloc");
        free(mail);
        exit(EXIT_FAILURE);
    }
    strcpy(toSave, INBOX);
    strcat(toSave, user);
    check_dir(toSave);
    strcat(toSave, "/");
    strcat(toSave, aux);
    char* com = malloc(strlen(argv[2]) + strlen(command) + strlen(toSave) + strlen(" 2> transform.err") + 5);
    snprintf(com, 300, "%s %s > %s 2> transform.err", command, mail, toSave);
    int transform = transform_mail(argv[2],command,toSave);
    free(toSave);
    if(transform == 0){
        write(STDOUT_FILENO, SUCCESS, strlen(SUCCESS) + 1);
    }

    ssize_t nbytes;
    char inputBuffer[MAX_BUFFER_SIZE];  
    while ((nbytes = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer) - 1)) > 0) {
        inputBuffer[nbytes] = '\0';  
        mail = malloc(strlen(inputBuffer) + 1);
        if (mail == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(mail, inputBuffer);

        user = strtok(mail, "-");
        aux = strtok(NULL, "-");
        aux[strlen(aux) -1 ] = '\0';

        toSave= malloc(strlen(INBOX) + strlen(aux) +strlen(user) + 2);
        
        strcpy(toSave, INBOX);
        strcat(toSave, user);
        check_dir(toSave);
        strcat(toSave, "/");
        strcat(toSave, aux);
        inputBuffer[nbytes - 1] = '\0';
    
        transform = transform_mail(inputBuffer,command,toSave);
        
        free(mail);
        free(toSave);
        if(transform == 0){
        write(STDOUT_FILENO, SUCCESS, strlen(SUCCESS) + 1);
        } else{
            write(STDOUT_FILENO,FAILURE,strlen(FAILURE)+1);
        }
    }
    return 0;
}
