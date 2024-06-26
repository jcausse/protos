/**
 * \file        transform.c
 * \brief       Transform mails.
 *
 * \author      De Caro, Guido
 */

#include "transform.h"
#include "../lib/logger.h"

#define TMP "./tmp"
#define INBOX "./inbox"
#define MODE_T 0770
#define SUCCESS 0
#define ERR -1
#define BUFF_SIZE 1024

extern Logger logger;

static void check_dir(char * dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, MODE_T);
    }
}

static int transform_mail(char * mail, char * command, char * toSave) {
    char * com = calloc(320,sizeof(char));
    snprintf(com, 320, "%s %s > %s 2> transform.err", command, mail, toSave);
    int retVal = system(com);
    free(com);
    return retVal;
}

static int send_mail(char * mail, char * toSave) {

    int mailFd = open(mail, 0 , MODE_T);
    int toSaveFd = open(toSave, O_CREAT | O_EXCL | O_WRONLY , MODE_T);

    if(mailFd == ERR || toSaveFd == ERR) return ERR;

    int n;
    struct stat s;
    off_t offset = 0;

    fstat(mailFd, &s);
    n = s.st_size;

    while(n > 0) {
        const int sb = sendfile(toSaveFd, mailFd, &offset, n);
        if(sb <= -1) {
            break;
        } else if(sb == 0) {
            break;
        } else {
           n -= sb;
        }
    }

    close(mailFd);
    close(toSaveFd);

    return SUCCESS;
}

int transform(bool enabled, char * cmd, char * mail, char * domain, char * user, char * s_name){
    check_dir(INBOX);

    char user_dir[BUFF_SIZE] = {0};
    char toSave[BUFF_SIZE] = {0};

    snprintf(user_dir, strlen(INBOX) + strlen(domain) + 2, "%s/%s", INBOX, domain);
    check_dir(user_dir);

    snprintf(user_dir, strlen(INBOX) + strlen(domain) + strlen(user) + 3, "%s/%s/%s", INBOX, domain, user);
    check_dir(user_dir);

    snprintf(toSave, strlen(INBOX) + strlen(domain) + strlen(user) + strlen(s_name) + 4, "%s/%s/%s/%s", INBOX, domain, user, s_name);
    int transform;
    if(enabled){
        transform = transform_mail(mail, cmd, toSave);
    }
    else {
        transform = send_mail(mail, toSave);
    }

    return transform != SUCCESS ? ERR : SUCCESS;
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
