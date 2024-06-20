#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int transform_mail(char * mail, char * command,char * toSave) {
    char com[300];
    strcat(com,command);
    strcat(com," ");
    strcat(com,mail);
    strcat(com," > ");
    strcat(com,toSave);
    strcat(com," 2> ");
    strcat(com,"transform.err");
    int retVal = system(com);
    return retVal;
}

int main (int argc, char *argv[]) {
    char * command = argv[1];
    char * mail = argv[2];
    char * toTransform = strtok(mail, ":");
    char * toSave = strtok(NULL, ":");

    int transform = transform_mail(toTransform,command,toSave);
    write(STDOUT_FILENO, &transform, sizeof(int));

    ssize_t nbytes;
    char inputBuffer[1024];  // Tamaño suficientemente grande para almacenar la entrada

    while ((nbytes = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer))) > 0) {
        toTransform = strtok(mail, ":");
        toSave = strtok(NULL, ":");
        transform = transform_mail(toTransform,command,toSave);
        char output[1];
        sprintf(output, "%d", transform);
        write(STDOUT_FILENO, output, sizeof(output));
    }

    return 0;
}
