/**
 * \file        main.c
 * \brief       SMTPD server main file. Starts SMTPD and starts
 *              listening for client connections.
 *
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#define MAX_BUFFER_SIZE 1049
#define TMP "./tmp"
#define INBOX "./inbox"
#define FILE_PERMISSIONS 0777


/****************************************************************/
/* Main function                                                */
/****************************************************************/

static void check_dir(char * dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }
}
/**
 * \brief                       Creates and runs the command to transform the given mail and leaves it in the inbox of the specified user.
 * 
 * \param[in] mail              Path of the mail to transform.
 * \param[in] command           Transformation command to apply.
 * \param[in] toSave            Path of the inbox to save the transformed mail.
 * 
 * \return                      On success, 0. On failure, -1.
 */
static int transform_mail(char * mail, char * command,char * toSave) {
    char* com = calloc(320,sizeof(char));
    snprintf(com, 320, "%s %s > %s 2> transform.err", command, mail, toSave);
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
int transform(char * cmd, char * mail,char* user,char* s_name) {
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
