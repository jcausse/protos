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
#include "smtpd_info.h" // Product and team information

// TODO REFACTOR
struct smtpd_args {
    char *          mail_directory;

    //char           *socks_addr;
    unsigned short  socks_port;

    //char *          mng_addr;
    unsigned short  mng_port;

    bool            disectors_enabled;

    //struct users    users[MAX_USERS];
};

// TODO REFACTOR
/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void 
parse_args(const int argc, char **argv, struct socks5args *args);

#endif

