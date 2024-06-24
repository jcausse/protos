/**
 * \file        args.h
 * \brief       Parse command-line arguments.
 */

#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <stdio.h>      // printf
#include <stdlib.h>     // exit
#include <limits.h>     // LONG_MIN, LONG_MAX
#include <string.h>     // memset
#include <stdint.h>     // uint16_t
#include <errno.h>      // errno
#include <getopt.h>     // getopt_long
#include <stdbool.h>    // bool 

// TODO REFACTOR
struct smtpd_args {
    char *          mail_directory; //Where the server is going to store the mails

    unsigned short  smtp_port;

    unsigned short  mng_port;

    char *          domain; //Which domain the server is going to be managing "example.com"

    char *          trsf_cmd;//Command for mail transformation

    char *          vrfy_mails;//Where to find the verified mails 

    bool            vryf_enabled;//Shall the server veryfy the mails

    bool            trsf_enabled;//Shall the server transform the mails
};

/**
 * \brief    Parse arguments
 * 
 * \param[in]
 */
bool parse_args(int argc, char ** argv, struct smtpd_args * const result);

/****************************************************************************/
#define PRODUCT_VERSION     "0.1.0"

#define ORGANIZATION        "ITBA, Protocolos de Comunicacion"

#define COMPILATION_DATE    __DATE__
#define COMPILATION_TIME    __TIME__

/****************************************************************************/

#define TEAM_NO "3"

#define TEAM_MEMBERS(XX)                                                    \
    XX("Causse",        "Juan Ignacio",     "61105")                        \
    XX("De Caro",       "Guido",            "61590")                        \
    XX("Mindlin",       "Felipe",           "62774")                        \
    XX("Sendot",        "Francisco",        "62351")                        

const char * team_members_last_names[] ={
    #define XX(LAST_NAME, FIRST_NAME, ID) LAST_NAME,
    TEAM_MEMBERS(XX)
    #undef XX
};

const char * team_members_first_names[] ={
    #define XX(LAST_NAME, FIRST_NAME, ID) FIRST_NAME,
    TEAM_MEMBERS(XX)
    #undef XX
};

const char * team_members_ids[] ={
    #define XX(LAST_NAME, FIRST_NAME, ID) ID,
    TEAM_MEMBERS(XX)
    #undef XX
};

#endif

