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

/*************************************************************************/
/* Include header files                                                  */
/*************************************************************************/

#include <limits.h>     // LONG_MIN, LONG_MAX, SHRT_MIN, SHRT_MAX, INT_MIN, INT_MAX
#include <string.h>     // memset
#include <stdint.h>     // uint16_t
#include <errno.h>      // errno
#include <getopt.h>     // getopt_long
#include <stdbool.h>    // bool

/*************************************************************************/
/* Typedefs                                                              */
/*************************************************************************/

/**
 * \typedef         SMTPDArgs: structure that contains all parsed arguments,
 *                  or its default values when not provided.
 */
typedef struct {
    char *          mail_directory; // Path to the directory where the server is going to store the mails.
    unsigned short  smtp_port;      // Port where the SMTP server will be listening to.
    unsigned short  mng_port;       // Port where the management server will be listening to.
    char *          domain;         // Domain the server is going to be managing ("example.com").
    char *          trsf_cmd;       // Command for mail transformation.
    char *          vrfy_mails;     // Where to find the verified mails.
    bool            vryf_enabled;   // Enables or disables verification.
    bool            trsf_enabled;   // Enables or disables transformation.
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

#endif // ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM81
