/**
 * \file        client_data.h
 * \brief       Handlers for all socket types used by the server.
 *
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 * \author      Sendot, Francisco
 */

#ifndef __CLIENT_DATA_H__
#define __CLIENT_DATA_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "parser.h"

#define WRITE_BUFF_SIZE 1400
#define READ_BUFF_SIZE 1400

#define TMP "./tmp"
#define INBOX "./inbox"
#define FILE_PERMISSIONS 0770

typedef struct _ClientData_t {
    Parser parser;

    char w_buff[WRITE_BUFF_SIZE];
    size_t w_count;

    char r_buff[WRITE_BUFF_SIZE];
    size_t r_count;

    char * clientDomain;

    char * senderMail;
    char * receiverMails;

    char * fileName;
    FILE *mailFile;
} _ClientData_t;

typedef struct _ClientData_t * ClientData;

#endif // __CLIENT_DATA_H__
