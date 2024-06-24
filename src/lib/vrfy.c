#include "vrfy.h"

int load_valid_mails(const char file_path, char ***valid_mails, int *mails) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL){
        perror("Failed to open file");
        return 0;
    }

    *mails = 0;
    *valid_mails = NULL;
    char line[MAX_LINE_LENGTH];
    while (fgets(line,sizeof(line),file)){
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        // Allocate memory for a new email
        *valid_mails = realloc(*valid_mails, (*mails + 1) * sizeof(char *));
        if (!*valid_mails){
            perror("Failed to allocate memory");
            fclose(file);
            return 0;
        }

        // Copy the email into the allocated memory
        (*valid_mails)[*mails] = strdup(line);
        if (!(*valid_mails)[*mails]){
            perror("Failed to duplicate string");
            fclose(file);
            return 0;
        }

        (*mails)++;
    }
    

    fclose(file);
    return 1;
}

int check_mail(const char *mail, char **valid_mails, int mails){
    for (int i = 0; i < mails; i++){
        if (strcmp(mail, valid_mails[i]) == 0){
            return SUCCESS;
        }
    }

    return FAILURE;
}

void free_valid_mails(char **valid_mails, int mails){
    for (int i = 0; i < mails; i++){
        free(valid_mails[i]);
    }
    free(valid_mails);
}

int vrfy(const char *mail,const char *file_path){
    char **valid_mails = NULL;
    int mail_count = 0;

    if(!load_valid_mails(&valid_mails,&mail_count,file_path)){
        fprintf(stderr,"Failed to load valid emails\n");
        exit(EXIT_FAILURE);
    }

    return vrfy_mail(vrfy_mail(mail,&valid_mails,mail_count));
}