#include "parser.h"
#include "vrfy.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFF_SIZE 256

char *strdup(const char *s);
static void clearLine(char *line);

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

    char line[BUFF_SIZE];
    while (fgets(line, BUFF_SIZE, file) != NULL) {
        if (strlen(line) <= queryLen || (strstr(line, query) == NULL)) {
            clearLine(line);
            continue;
        }

        // Allocate memory for a new email
        *validMails = realloc(*validMails, (*mailCount + 1) * sizeof(char *));
        if (!*validMails) {
            // Use logger to log error
            freeValidMails(*validMails, *mailCount);
            fclose(file);
            return ERR;
        }

        // Copy the email into the allocated memory
        (*validMails)[*mailCount] = strdup(line);
        if (!(*validMails)[*mailCount]) {
            // Use logger to log error
            freeValidMails(*validMails, *mailCount);
            fclose(file);
            return ERR;
        }
        clearLine(line);
        (*mailCount)++;
    }
    if(*mailCount < 1) {
        freeValidMails(*validMails, *mailCount);
        return ERR;
    }
    fclose(file);
    return SUCCESS;
}
static void clearLine(char *line) {
    for(int i = 0; i < BUFF_SIZE || line[i] != '\0'; i++) line[i] = '\0';
}
