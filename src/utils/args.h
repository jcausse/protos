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
#define PRODUCT_VERSION     "0.1.0"

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

/**
 * \typedef         SMTPDArgs: structure that contains all parsed arguments,
 *                  or its default values when not provided.
 */
typedef struct {
    char *      domain;             // Domain the server is going to be managing ("example.com").
    uint16_t    smtp_port;          // Port where the SMTP server will be listening to.
    uint16_t    mngr_port;          // Port where the management server will be listening to.
    char *      trsf_cmd;           // Command for mail transformation.
    char *      vrfy_mails;         // Where to find the verified mails.
    bool        vryf_enabled;       // Enables or disables verification.
    bool        trsf_enabled;       // Enables or disables transformation.

    /**
     * Minimum log level
     * 
     * required:    false
     * default:     LOGGER_DEFAULT_MIN_LOG_LEVEL
     * options:     LOGGER_LEVEL_DEBUG, LOGGER_LEVEL_INFO, LOGGER_LEVEL_NORMAL, LOGGER_LEVEL_CRITICAL
     */
    LogLevels   min_log_level;
} SMTPDArgs;

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
bool parse_args(int argc, char ** argv, SMTPDArgs * const result);
void usage(const char *progname);
void version(void);
short parse_short(const char* str, int radix);
long parse_long(const char* str, int radix);

#endif // ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM81
