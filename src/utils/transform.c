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
#define MAX_DIR_SIZE 512 // Out file system has a 2-level directory to save the mails
#define MAIL_FROM_STR "MAIL FROM: <%s>\r\n"
#define RCPT_TO_STR "RCPT TO: <%s>\r\n"
#define DATA_STR "DATA\r\n"
#define DOT_CLRF ".\r\n"

extern Logger logger;

static void check_dir(char * dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, MODE_T);
    }
}

int transform(char * cmd, char * mailDir) {

    char buff[BUFF_SIZE] = {0};

    for(int i = 0; i < BUFF_SIZE && buff[i] != '\0'; i++) buff[i] = '\0';
    snprintf(buff, 320, "%s '%s' > '%s'", cmd, mailDir, mailDir);

    int retVal = system(buff);

    return retVal;
}

static int send_mail(char * mailDir, char * receiverMail, char * senderMail,char * toSave) {
    int mailFd = open(mailDir, 0 , MODE_T);
    int toSaveFd = open(toSave, O_CREAT | O_EXCL | O_WRONLY , MODE_T);

    if(mailFd == ERR || toSaveFd == ERR) return ERR;

    char buff[BUFF_SIZE] = {0};

    sprintf(buff, MAIL_FROM_STR, senderMail);
    write(toSaveFd, buff, strlen(buff));
    for(int i = 0; i < BUFF_SIZE && buff[i] != '\0'; i++) buff[i] = '\0';

    sprintf(buff, RCPT_TO_STR, receiverMail);
    write(toSaveFd, buff, strlen(buff));

    write(toSaveFd, DATA_STR, strlen(DATA_STR));

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

    write(toSaveFd, DOT_CLRF, strlen(DOT_CLRF));

    close(mailFd);
    close(toSaveFd);

    return SUCCESS;
}

int dump(char * mailDir, char * receiverMail, char * senderMail, char * fileName){
    char userName[MAX_DIR_SIZE/2] = {0};
    char domain[MAX_DIR_SIZE/2] = {0};
    int receiverLen = strlen(receiverMail);
    int i = 0;
    while(i < receiverLen && receiverMail[i] != '@'){
        userName[i] = receiverMail[i];
        i++;
    }
    i++;
    int j = 0;
    while(i < receiverLen && receiverMail[i] != '\r' && receiverMail[i] != '\n' && receiverMail[i] != '\0' ){
        domain[j] = receiverMail[i];
        i++;
        j++;
    }
    check_dir(INBOX);

    char user_dir[BUFF_SIZE] = {0};
    char toSave[BUFF_SIZE] = {0};

    snprintf(user_dir, strlen(INBOX) + strlen(domain) + 2, "%s/%s", INBOX, domain);
    check_dir(user_dir);

    snprintf(user_dir, strlen(INBOX) + strlen(domain) + strlen(userName) + 3, "%s/%s/%s", INBOX, domain, userName);
    check_dir(user_dir);

    snprintf(toSave, strlen(INBOX) + strlen(domain) + strlen(userName) + strlen(fileName) + 4, "%s/%s/%s/%s", INBOX, domain, userName, fileName);

    int transform = send_mail(mailDir, receiverMail, senderMail, toSave);

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
