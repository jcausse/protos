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

#define MSG_ERR_EACCES              "Error creating or opening log file: %s. Permissions needed."

/********************************************************/
/* Exit messages                                        */
/********************************************************/

#define MSG_EXIT_SIGINT             "Received SIGINT. Cleaning up and exiting."
#define MSG_EXIT_FAILURE            "SMTPD exited due to an unexpected error."

/********************************************************/
/* Error messages                                       */
/********************************************************/

#define MSG_ERR_REGEX_COMPILATION   "Could not compile SMTP parser regexes."
#define MSG_ERR_SV_SOCKET           "Could not create server socket."
#define MSG_ERR_MNGR_SOCKET         "Could not create management socket."
#define MSG_ERR_STATS_CREATION      "Could not initialize statistics."
#define MSG_ERR_SELECTOR_CREATION   "Could not create Selector."
#define MSG_ERR_NO_MEM              "Could not allocate memory."
#define MSG_ERR_SELECT              "select (2) error."
#define MSG_ERR_UNK_SOCKET_TYPE     "Socket %d reported unknown type %d."

/********************************************************/
/* Normal log messages                                  */
/********************************************************/

#define MSG_SERVER_STARTED          "Server started."
#define MSG_NEW_CLIENT              "New client connected at %s : %d."

/********************************************************/
/* Verbose log messages                                 */
/********************************************************/

#define MSG_INFO_LOGGER_CREATED     "Logger started."
#define MSG_INFO_REGEX_COMPILED     "Compiled SMTP parser regexes."
#define MSG_INFO_SV_SOCKET_CREATED  "Listening for SMTP connections on TCP port %d."
#define MSG_INFO_MNG_SOCKET_CREATED "Listening for management connections on UDP port %d."
#define MSG_INFO_STATS_CREATED      "Statistics initialized."
#define MSG_INFO_SELECTOR_CREATED   "Selector started."
#define MSG_INFO_BAD_MNGR_COMMAND   "Manager sent an invalid command."
#define MSG_INFO_MNGR_COMMAND       "Manager sent command %s (%02X)"

/********************************************************/
/* Debug log messages                                   */
/********************************************************/

#define MSG_DEBUG_SELECTOR_ADD      "Added fd %d (type %d) to Selector."
#define MSG_DEBUG_SELECTOR_SELECT   "Performing select operation."
#define MSG_DEBUG_SOCKET_READY      "Fd %d (type %d) is ready for %s operation."

#endif // __MESSAGES_H__
