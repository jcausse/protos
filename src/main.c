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
#include "args.h"

/****************************************************************/
/* SMTPD configuration                                          */
/****************************************************************/

/**
 * \todo esto deberia salir de la configuracion
 */
#define CONFIG_PORT             2525
#define CONFIG_BACKLOG_SIZE     10
#define CONFIG_LOG_FILE         "/home/juani/Desktop/smtpd.log"
#define CONFIG_LOG_LEVEL        LOGGER_LEVEL_DEBUG

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
 * \param[in] args      Command-line  arguments
 */
static void smtpd_init(SMTPDArgs * args);

/**
 * \brief       Starts SMTPD after initialization. This function only returns if an error occurs.
 */
static void smtpd_start(void);

/**
 * \brief       Cleanup resources and exit with code `exit_code`.
 * 
 * \param[in] exit_code Integer passed to exit (3).
 */
static void smtpd_cleanup(int exit_code);

/**
 * \brief       Abort execution. Performs cleanup and exites with code `EXIT_FAILURE`.
 */
static void smtpd_abort(void);

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
    /* Parse command-line arguments */
    SMTPDArgs args;
    if (! parse_args(argc, argv, &args)){
        return EXIT_FAILURE;
    }

    // \todo: pasar los argumentos a donde corresponda

    /* Initialize and start server */
    smtpd_init(&args);                  // Initialize SMTPD.
    smtpd_start();                      // Start SMTPD. Only returns on error.
    smtpd_abort();                      // Cleanup on error.
    return EXIT_FAILURE;                // Never reached.
}

/****************************************************************/
/* Private function definitions                                 */
/****************************************************************/

static void smtpd_init(SMTPDArgs * const args){
    (void) args; // \todo usar esta variable
    /* Close unused file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);

    /* Set SIGINT handler */
    signal(SIGINT, sigint_handler);

    /* Variables */
    int         sv_fd_4     = -1;       // IPv4 server socket
    int         sv_fd_6     = -1;       // IPv6 server socket

    /* Logger configuration */
    LoggerConfig logger_cfg = {
        .min_log_level      = CONFIG_LOG_LEVEL, // Minimum log level
        .with_datetime      = true,     // Include date and time in logs
        .with_level         = true,     // Include log levels
        .flush_immediately  = true,     // Disable buffering for real-time log viewing (tail -f)
        .log_prefix         = NULL      // "smtpd v1.0.0" \todo
    };

    TRY{
        /* Create Logger */
        THROW_IF(
            (logger =
                Logger_create(
                    logger_cfg,         // Logger configuration
                    CONFIG_LOG_FILE     // Absolute path to the file that will hold the logs
                )
            ) == NULL                   // Expected return: Logger (not NULL)
        );
        LOG_VERBOSE(MSG_INFO_LOGGER_CREATED);

        /* Close stderr as it will not be used */
        close(STDERR_FILENO);

        /* Create passive sockets (server sockets) for IPv4 and IPv6 */
        THROW_IF_NOT(
            tcp_serve(
                CONFIG_PORT,            // Port for SMTPD
                CONFIG_BACKLOG_SIZE,    // Max quantity of pending (unaccepted) connections
                &sv_fd_4,               // IPv4 socket (output parameter)
                &sv_fd_6                // IPv6 socket (output parameter)
            )
        );                              // Expected return: true
        LOG_VERBOSE(MSG_INFO_SV_SOCKET_CREATED, CONFIG_PORT);

        /* Create Selector */
        THROW_IF((selector = Selector_create(free)) == NULL) // \todo data free fn
        LOG_VERBOSE(MSG_INFO_SELECTOR_CREATED);

        /* Add both of the server sockets to the Selector */
        THROW_IF_NOT(
            Selector_add(
                selector,               // The Selector itself
                sv_fd_4,                // File descriptor to add
                SELECTOR_READ,          // Mode
                SOCK_TYPE_SERVER4,      // File descriptor type
                NULL                    // No data needed
            )
            == SELECTOR_OK              // Expected return: SELECTOR_OK
        );
        LOG_DEBUG(MSG_DEBUG_SELECTOR_ADD, sv_fd_4, SOCK_TYPE_SERVER4);
        THROW_IF_NOT(
            Selector_add(
                selector,               // The Selector itself
                sv_fd_6,                // File descriptor to add
                SELECTOR_READ,          // Mode
                SOCK_TYPE_SERVER6,      // File descriptor type
                NULL                    // No data needed
            )
            == SELECTOR_OK              // Expected return: SELECTOR_OK
        );
        LOG_DEBUG(MSG_DEBUG_SELECTOR_ADD, sv_fd_6, SOCK_TYPE_SERVER6);
    }
    CATCH{
        /* Could not create the logger */
        if (logger == NULL){
            if (errno == EACCES){
                fprintf(stderr, MSG_ERR_EACCES, CONFIG_LOG_FILE);
            }
            else if (errno == ENOMEM){
                fprintf(stderr, MSG_ERR_NO_MEM);
            }
            fprintf(stderr, MSG_EXIT_FAILURE);
        }

        /* Could not create server socket */
        else if (sv_fd_4 == -1 || sv_fd_6 == -1){
            LOG_ERR(MSG_ERR_SV_SOCKET);
        }

        /* No memory available for allocation */
        else{
            LOG_ERR(MSG_ERR_NO_MEM);
        }

        /* Cleanup and exit */
        safe_close(sv_fd_4);
        safe_close(sv_fd_6);
        smtpd_abort();
    }
}

static void smtpd_start(void){
    while (true){
        /* Perform a select (2) operation */
        LOG_DEBUG(MSG_DEBUG_SELECTOR_SELECT);
        SelectorErrors err = Selector_select(selector);     // Blocking
        if (err != SELECTOR_OK){
            if (err == SELECTOR_SELECT_ERR){
                LOG_ERR(MSG_ERR_SELECT);
            }
            smtpd_abort();
        }
        
        /* Iterate through all ready file descriptors */
        int     sock_fd;
        int     sock_type;
        void *  sock_data;
        while ((sock_fd = Selector_read_next(selector, &sock_type, &sock_data)) != SELECTOR_NO_FD){
            if (sock_type < 0 || sock_type >= SOCK_TYPE_QTY){
                Selector_remove(selector, sock_fd, SELECTOR_READ_WRITE, true);
                LOG_ERR(MSG_ERR_UNK_SOCKET_TYPE, sock_fd, sock_type);
                continue;
            }
            LOG_DEBUG(MSG_DEBUG_SOCKET_READY, sock_fd, sock_type, "READ");
            read_handlers[sock_type](sock_fd, sock_data);
        }
        while ((sock_fd = Selector_write_next(selector, &sock_type, &sock_data)) != SELECTOR_NO_FD){
            if (sock_type < 0 || sock_type >= SOCK_TYPE_QTY){
                Selector_remove(selector, sock_fd, SELECTOR_READ_WRITE, true);
                LOG_ERR(MSG_ERR_UNK_SOCKET_TYPE, sock_fd, sock_type);
                continue;
            }
            LOG_DEBUG(MSG_DEBUG_SOCKET_READY, sock_fd, sock_type, "WRITE");
            write_handlers[sock_type](sock_fd, sock_data);
        }
    }
}

static void smtpd_cleanup(int exit_code){
    Selector_cleanup(selector);     // NULL-safe
    Logger_cleanup(logger);         // NULL-safe
    exit(exit_code);
}

static void smtpd_abort(void){
    LOG_ERR(MSG_EXIT_FAILURE);
    smtpd_cleanup(EXIT_FAILURE);
}

void sigint_handler(int signum){
    (void) signum;                  // Avoids unused parameter warning
    LOG_MSG(MSG_EXIT_SIGINT);       // NULL-safe
    smtpd_cleanup(EXIT_SUCCESS);    // No error occurred
}
