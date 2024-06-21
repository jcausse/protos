/**
 * \file        messages.h
 * \brief       Logger messages
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */
#ifndef __MESSAGES_H__
#define __MESSAGES_H__

/********************************************************/
/* Pre-Logger error messages                            */
/********************************************************/

#define MSG_ERR_EACCES          "Error creating or opening log file: %s. Permissions needed."

/********************************************************/
/* Exit messages                                        */
/********************************************************/

#define MSG_EXIT_SIGINT         "Received SIGINT. Cleaning up and exiting."
#define MSG_EXIT_FAILURE        "SMTPD exited due to an unexpected error."

/********************************************************/
/* Error messages                                       */
/********************************************************/

#define MSG_ERR_SV_SOCKET       "Error opening server socket."
#define MSG_ERR_NO_MEM          "Could not allocate memory."

#endif // __MESSAGES_H__
