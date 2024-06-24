/**
 * \file        smtpd_info.h
 * \brief       Information about the product and its authors.
 */

#ifndef __SMTPD_INFO_H__
#define __SMTPD_INFO_H__

#include <stdlib.h>

/****************************************************************************/

#define PRODUCT_NAME        "SMTPd"
#define PRODUCT_VERSION     "0.1.0"

#define ORGANIZATION        "ITBA, Protocolos de Comunicacion"

#define COMPILATION_DATE    __DATE__
#define COMPILATION_TIME    __TIME__

/****************************************************************************/

#define TEAM_NO "1"

#define TEAM_MEMBERS(XX)                                                    \
    XX("Causse",        "Juan Ignacio",     "61105")                        \
    XX("De Caro",       "Guido",            "61590")                        \
    XX("Mindlin",       "Felipe",           "62774")                        \
    XX("Sendot",        "Francisco",        "62351")                        \
    XX(NULL,            NULL,               NULL   )

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

/****************************************************************************/

#endif // __SMTPD_INFO_H__
