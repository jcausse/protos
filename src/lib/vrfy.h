#ifndef VRFY_H
#define VRFY_H

#define MAX_EMAIL_LENGTH 256
#define MAX_LINE_LENGTH 512
#define SUCCESS 250
#define FAILURE 550
#define VALID_FILE "vaild_mails.txt"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * \brief                   Loads all mails from the file into the valid mails array.
 * \param[in] file_path     Path to the file containing the valid mails.
 * \param[in] valid_mails   Array of mils to be freed.
 * \param[in] mails         Number of mails in the array.
 * \return                  1 if the mails were loaded successfully, 0 if not.
*/
int load_valid_mails(const char file_path,char ***valid_mails, int *mails);

/**
 * \brief                   check if the mail is in the valid mails list.
 * \param[in] mail          Mail to be checked.
 * \param[in] valid_mails   Array of mils to be freed.
 * \param[in] mails         Number of mails in the array.
 * \return                  250 if the mail is valid, 550 if it is not.
*/
int check_mail(const char *email, char **valid_emails, int count);

/**
 * \brief                   Free memory allocated for the valid mails.
 * \param[in] valid_mails   Array of mils to be freed.
 * \param[in] mails         Number of mails in the array.
 * \return                  void
*/
void free_valid_mails(char **valid_mails, int mails);

/**
 * \brief                   Checks if the email is a verified email.
 * \param[in] Mail          Mail the user wants to check if it is verified.
 * \param[in] file_path     Path to the file containing the valid mails.
 * \return                  250 if the mail is verified, 550 if it is not.
*/
int vrfy(const char *mail,const char *file_path);
#endif // VRFY_H
