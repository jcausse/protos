/**
 * \file        args.h
 * \brief       Parse command-line arguments.
 * 
 * \author      Causse, Juan Ignacio
 * \author      Codagnone, Juan Francisco
 * \author      De Caro, Guido
 */

#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM81
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM81

#define PRODUCT_NAME        "smtpd"
#define PRODUCT_VERSION     "0.0.0"

/*************************************************************************/
/* Include header files                                                  */
/*************************************************************************/

#include <stdio.h>          // fprintf(), stderr
#include <limits.h>         // LONG_MIN, LONG_MAX, SHRT_MIN, SHRT_MAX, INT_MIN, INT_MAX
#include <string.h>         // memset
#include <stdint.h>         // uint16_t
#include <errno.h>          // errno
#include <getopt.h>         // getopt_long
#include <stdbool.h>        // bool
#include "../lib/logger.h"  // LogLevels

/*************************************************************************/
/* Typedefs                                                              */
/*************************************************************************/


/*************************************************************************/
/* Public function prototypes                                            */
/*************************************************************************/

/**
 * \brief    Parse arguments from command-line.
 * 
 * \param[in]  argc         Argument count (includes executable name).
 * \param[in]  argv         Argument string array (includes executable name).
 * \param[out] result       Pointer to structure smtpd_args used to save parsed arguments and values.
 * 
 * \return  Returns true on success, false otherwise.
 */

typedef struct {
    char *server_ip;
    int port;
} UDPArgs;

bool parse_args(int argc, char **argv, UDPArgs *const result);
void usage(const char *progname);
void version(void);
short parse_short(const char* str, int radix);
long parse_long(const char* str, int radix);

#endif // ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM81
