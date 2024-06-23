/**
 * \file        main.c
 * \brief       SMTPD server main file. Starts SMTPD and starts 
 *              listening for client connections.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#include <stdio.h>
#include <signal.h>

#include "logger.h"
#include "selector.h"
#include "sockets.h"
#include "sock_types_handlers.h"
#include "exceptions.h"
#include "messages.h"

/****************************************************************/
/* SMTPD configuration                                          */
/****************************************************************/

/**
 * \todo esto deberia salir de la configuracion
 */
#define SMTPD_PORT      2222
#define BACKLOG_SIZE    10
#define LOG_FILE "/home/juani/Desktop/smtpd.log"

/****************************************************************/
/* Global variables                                             */
/****************************************************************/

Logger      logger      = NULL;     // Logger (see src/lib/logger.h)
Selector    selector    = NULL;     // Selector (see src/utils/selector.h)

/****************************************************************/
/* Extern global variables                                      */
/****************************************************************/

extern SockReadHandler  read_handlers[];
extern SockWriteHandler write_handlers[];

/****************************************************************/
/* Private function declarations                                */
/****************************************************************/

/**
 * \brief       Initializes SMTPD.
 * 
 * \return      true if successfully initialized, false otherwise.
 */
bool smtpd_init();

/**
 * \brief       Starts SMTPD after initialization.
 * 
 * \return      On success, this function does not return. On failure, returns false.
 */
bool smtpd_start();

/**
 * \brief       Captures SIGINT signal to gracefully stop SMTPD.
 * \details     Attempts to perform a cleanup of the Selector and the Logger.
 * 
 * \param[in] signum    According to the documentation, the handler receives the
 *                      signal number that triggered its call. In this case, SIGINT.
 */
void sigint_handler(int sigint);

/****************************************************************/
/* Main function                                                */
/****************************************************************/

int main(int argc, char ** argv){
    /**
     * \todo argumentos
     */
    (void) argc;
    (void) argv;
    if (smtpd_init()){
        smtpd_start();
    }
    return EXIT_FAILURE;
}

/****************************************************************/
/* Private function definitions                                 */
/****************************************************************/

bool smtpd_init(){
    /* Set SIGINT handler */
    signal(SIGINT, sigint_handler);

    /* Variables */
    int         sv_fd_4     = -1;       // IPv4 server socket
    int         sv_fd_6     = -1;       // IPv6 server socket

    /* Logger configuration */
    LoggerConfig logger_cfg = {
        .min_log_level      = LOGGER_DEFAULT_MIN_LOG_LEVEL, // Minimum log level
        .with_datetime      = true,     // Include date and time in logs
        .with_level         = true,     // Include log levels
        .flush_immediately  = true,     // Disable buffering for real-time log viewing (tail -f)
        .log_prefix         = "smtpd v1.0.0"
    };

    TRY{
        /* Create Logger */
        THROW_IF(
            (logger =
                Logger_create(
                    logger_cfg,         // Logger configuration
                    LOG_FILE            // Absolute path to the file that will hold the logs
                )
            ) == NULL                   // Expected return: Logger (not NULL)
        );

        /* Create passive sockets (server sockets) for IPv4 and IPv6 */
        THROW_IF_NOT(
            tcp_serve(
                SMTPD_PORT,             // Port for SMTPD
                BACKLOG_SIZE,           // Max quantity of pending (unaccepted) connections
                &sv_fd_4,               // IPv4 socket (output parameter)
                &sv_fd_6                // IPv6 socket (output parameter)
            )
        );                              // Expected return: true

        /* Create Selector */
        THROW_IF((selector = Selector_create(free)) == NULL)

        /* Add both of the server sockets to the Selector */
        THROW_IF_NOT(
            Selector_add(
                selector,               // The Selector itself
                sv_fd_4,                // File descriptor to add
                SELECTOR_READ,          // Mode
                SOCK_TYPE_SERVER,       // File descriptor type
                NULL                    // No data needed
            )
            == SELECTOR_OK              // Expected return: SELECTOR_OK
        );
        THROW_IF_NOT(
            Selector_add(
                selector,               // The Selector itself
                sv_fd_6,                // File descriptor to add
                SELECTOR_READ,          // Mode
                SOCK_TYPE_SERVER,       // File descriptor type
                NULL                    // No data needed
            )
            == SELECTOR_OK              // Expected return: SELECTOR_OK
        );
    }
    CATCH{
        /* Could not create the logger*/
        if (logger == NULL){
            if (errno == EACCES){
                fprintf(stderr, MSG_ERR_EACCES, LOG_FILE);
            }
            else if (errno == ENOMEM){
                fprintf(stderr, MSG_ERR_NO_MEM);
            }
            fprintf(stderr, MSG_EXIT_FAILURE);
            return false;
        }

        /* Could not create server socket */
        else if (sv_fd_4 == -1 || sv_fd_6 == -1){
            LOG_ERR(MSG_ERR_SV_SOCKET);
        }

        /* No memory available for allocation */
        else{
            LOG_ERR(MSG_ERR_NO_MEM);
        }

        /* Log that SMTPD exited on error */
        LOG_ERR(MSG_EXIT_FAILURE);

        /* Cleanup and exit */
        safe_close(sv_fd_4);
        safe_close(sv_fd_6);
        Selector_cleanup(selector);
        Logger_cleanup(logger);
        return false;
    }

    return true;
}

bool smtpd_start(){
    while (true) {
        
    }

    return false;
}

void sigint_handler(int signum){
    (void) signum;                  // Avoids unused parameter warning
    LOG_MSG(MSG_EXIT_SIGINT);       // NULL-safe
    Selector_cleanup(selector);     // NULL-safe
    Logger_cleanup(logger);         // NULL-safe
    exit(EXIT_SUCCESS);             // No error
}
