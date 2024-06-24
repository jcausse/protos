#include "parser.h"
#include "vrfy.h"

void freeValidMails(char **validMails, int mails){
    for (int i = 0; i < mails; i++){
        free(validMails[i]);
    }
    free(validMails);
}

int vrfy(const char *query, const char *filePath, char *** validMails, int *mailCount){
    FILE *file = fopen(filePath, "r");
    if (file == NULL){
        // Use logger to print error
        return ERR;
    }

    *mailCount = 0;
    *validMails = (char **) malloc(sizeof(char *));

    unsigned long queryLen = strlen(query);

    char *line = NULL;
    size_t *mailLen;
    while (getline(&line, mailLen, file) != ERR){
        // If the mail is shorter than the query is not necessary to compare them
        if(*mailLen < queryLen || (strncmp(query, line, queryLen) != SUCCESS)) {
            free(mailLen);
            free(line);
            continue;
        };

        // Allocate memory for a new email
        *validMails = realloc(*validMails, (*mailCount + 1) * sizeof(char *));
        if (!*validMails){
            // Use logger to log error
            freeValidMails(*validMails, *mailCount);
            free(line);
            free(mailLen);
            fclose(file);
            return ERR;
        }

        // Copy the email into the allocated memory
        (*validMails)[*mailCount] = strdup(line);
        if (!(*validMails)[*mailCount]){
            // Use logger to log error
            freeValidMails(*validMails, *mailCount);
            free(line);
            free(mailLen);
            fclose(file);
            return ERR;
        }
        free(line);
        free(mailLen);
        (*mailCount)++;
    }

    free(line);
    free(mailLen);
    fclose(file);
    return SUCCESS;
}
