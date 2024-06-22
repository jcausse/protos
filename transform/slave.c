#include "transform_central.h"
#define ERR_MSG "Usage: <command> <mail>\n"

void check_dir(char * dir){
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }
}

void removeSubstr (char *string, char *sub){
    char *match;
    int len = strlen(sub);
    while ((match = strstr(string, sub))) {
        *match = '\0';
        strcat(string, match+len);
    }
}

int transform_mail(char * mail, char * command,char * toSave){
    char* com = calloc(300,sizeof(char));
    removeSubstr(toSave, TO_TRANSFORM);
    snprintf(com, 300, "%s %s > %s 2> transform.err", command, mail, toSave);
    printf("COM: %s\n",com);
    int retVal = system(com);
    free(com);
    return retVal;  
}

/**
 * \brief                       Parses the file given to extract the user and the mail to transform.
 * 
 * \param[in] argc              Name of the program, name of the transformation command and the path to the mail to transform.
 * \param[in] argv              The arguments mentiones above in that order
 * .
 * 
 * \return                      On success, 254. On failure, 255 via pipe.
 */
int main (int argc, char *argv[]) {
    if (argc < 2) {
        printf(ERR_MSG);
    }
    check_dir(INBOX);
    char * command = argv[1];
    char * mail;
    char * user;
    char * aux;
    char * toSave;
    int transform;
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
        removeSubstr(user, TO_TRANSFORM);
        puts(user);
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
