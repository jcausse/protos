#ifndef VRFY_H
#define VRFY_H

#define MAX_EMAIL_LENGTH 256
#define MAX_LINE_LENGTH 512
#define VALID_FILE "vaild_mails.txt"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * \brief                   Free memory allocated for the valid mails.
 * \param[in] valid_mails   Array of mils to be freed.
 * \param[in] mails         Number of mails in the array.
 * \return                  void
*/
void freeValidMails(char **validMails, int mails);

/**
 * \brief                   Checks if the email is a verified email.
 * \param[in] mail          Mail the user wants to check if it is verified.
 * \param[in] filePath      Path to the file containing the valid mails.
 * \param[in] validMails    Pointer will the list of valid mails will be allocated.
 * \param[in] mailCount     Pointer to the final mails amount encoutered.
 * \return                  250 if the mail is verified, 550 if it is not.
*/
int vrfy(const char *mail, const char *filePath, char *** validMails, int *mailCount);

#endif // VRFY_H
