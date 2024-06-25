/**
 * \file        transform.h
 * \brief       Transform mails.
 *
 * \author      De Caro, Guido
 */

#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include "../lib/logger.h"

/**
 * \brief                       Parses the file given to extract the user and the mail to transform.
 *
 * \param[in] argc              Name of the program, name of the transformation command and the path to the mail to transform.
 * \param[in] argv              The arguments mentiones above in that order
 * .
 *
 * \return                      On success, 254. On failure, 255 via pipe.
 */
int transform(bool enabled, char * cmd, char * mailDir, char * domain, char * user, char * s_name);

#endif // __TRANSFORM_H__
