#include "parser.h"
#include "vrfy.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void freeValidMails(char **validMails, int mails) {
    for (int i = 0; i < mails; i++) {
        free(validMails[i]);
    }
    free(validMails);
}

int vrfy(const char *query, const char *filePath, char ***validMails, int *mailCount) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        // Use logger to print error
        return ERR;
    }

    *mailCount = 0;
    *validMails = (char **)malloc(sizeof(char *));

    unsigned long queryLen = strlen(query);

    char *line = NULL;
    size_t mailLen = 0;
    while (getline(&line, &mailLen, file) != ERR) {
        if (mailLen < queryLen || (strncmp(query, line, queryLen) != SUCCESS)) {
            free(line);
            line = NULL; // Reset the line pointer to NULL for the next getline call
            continue;
        }

        // Allocate memory for a new email
        *validMails = realloc(*validMails, (*mailCount + 1) * sizeof(char *));
        if (!*validMails) {
            // Use logger to log error
            freeValidMails(*validMails, *mailCount);
            free(line);
            fclose(file);
            return ERR;
        }

        // Copy the email into the allocated memory
        (*validMails)[*mailCount] = strdup(line);
        if (!(*validMails)[*mailCount]) {
            // Use logger to log error
            freeValidMails(*validMails, *mailCount);
            free(line);
            fclose(file);
            return ERR;
        }
        free(line);
        line = NULL; // Reset the line pointer to NULL for the next getline call
        (*mailCount)++;
    }

    free(line);
    fclose(file);
    return SUCCESS;
}