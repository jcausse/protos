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

static void check_dir(char * dir){
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }
}

int transform_mail(char * mail, char * command,char * toSave) {
    char* com = calloc(300,sizeof(char));
    strcat(com,command);
    strcat(com," ");
    strcat(com,mail);
    strcat(com," > ");
    strcat(com,toSave);
    strcat(com," 2> ");
    strcat(com,"transform.err");
    int retVal = system(com);
    free(com);
    return retVal;  
}

int main (int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <command> <mail>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    check_dir(INBOX);
    char * command = argv[1];
    char * mail = malloc(strlen(argv[2]) + 1);
    strcpy(mail, argv[2]);
    
  
    char * user = strtok(mail, "-");
    char * aux = strtok(NULL, "-");

    char * toSave= malloc(strlen(INBOX) + strlen(aux) +strlen(user) + 2);
    strcpy(toSave, INBOX);
    strcat(toSave, user);
    check_dir(toSave);
    strcat(toSave, "/");
    strcat(toSave, aux);

    int transform = transform_mail(argv[2],command,toSave);
    free(toSave);
    if(transform == 0){
        write(STDOUT_FILENO, SUCCESS, strlen(SUCCESS) + 1);
    }

    ssize_t nbytes;
    char inputBuffer[MAX_BUFFER_SIZE];  
    while ((nbytes = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer))) > 0) {
       if (nbytes > MAX_BUFFER_SIZE) {
            fprintf(stderr, "Input too long\n");
            continue;
        }

        inputBuffer[nbytes] = '\0';  

        mail = malloc(strlen(inputBuffer) + 1);
        if (mail == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(mail, inputBuffer);
        user = strtok(mail, "-");
        aux = strtok(NULL, "-");
        toSave= malloc(strlen(INBOX) + strlen(aux) +strlen(user) + 2);
        strcpy(toSave, INBOX);
        strcat(toSave, user);
        check_dir(toSave);
        strcat(toSave, "/");
        strcat(toSave, aux);
        transform = transform_mail(inputBuffer,command,toSave);
        
        free(mail);
        if(transform == 0){
        write(STDOUT_FILENO, SUCCESS, strlen(SUCCESS) + 1);
        }  // Free the allocated memory for mail after use
    }
    return 0;
}
