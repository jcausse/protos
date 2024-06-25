/**
 * \file        transform.c
 * \brief       Transform mails.
 *
 * \author      De Caro, Guido
 */

#include "transform.h"

#define TMP "./tmp"
#define INBOX "./inbox"
#define FILE_PERMISSIONS 0777

extern Logger logger;

static void check_dir(char * dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }
}

static int transform_mail(char * mail, char * command,char * toSave) {
    char* com = calloc(320,sizeof(char));
    snprintf(com, 320, "%s %s > %s 2> transform.err", command, mail, toSave);
    int retVal = system(com);
    free(com);
    return retVal;  
}

int transform(char * cmd, char * mail,char* user,char* s_name){
    char *command = cmd;
    check_dir(INBOX);
    char * toSave;
    char * user_dir = malloc(strlen(INBOX)+strlen(user)+2);
    
    toSave = malloc(strlen(INBOX) + strlen(s_name) + strlen(user) + 4);
    snprintf(user_dir,strlen(INBOX)+strlen(user)+2,"%s/%s",INBOX,user);
    check_dir(user_dir);
    snprintf(toSave, strlen(INBOX) + strlen(user) + strlen(s_name) + 4, "%s/%s/%s", INBOX, user, s_name);
    int transform = transform_mail(mail, command, toSave);
    free(toSave);

    if (transform == 0) {
        printf("Success transforming mail\n");
        return 254;
    } else {
        printf("Error transforming mail\n");
        return 255;
    }
    
    return 0;
}

#if 0
int main(void){
    mkdir(TMP, FILE_PERMISSIONS);
    mkdir(INBOX, FILE_PERMISSIONS);
    char * cmd = "cat";
    char * file_name = "./tmp/Ayerjojo.txt";
    char * user = "pepito";
    char * save_name = "Ayerjojo.txt";
    int trsf;
    /*If there is a new mail to transform*/
    trsf = transform(cmd,file_name, user,save_name);
    printf("%d\n",trsf); 
    return 1;  
}
#endif
