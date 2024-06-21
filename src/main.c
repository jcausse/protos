/**
 * \file        main.c
 * \brief       SMTPD server main file. Starts SMTPD and starts 
 *              listening for client connections.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#include "selector.h"
#include "sockets.h"
#include "sock_types_handlers.h"
#include "exceptions.h"

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
/* Private function declarations                                */
/****************************************************************/

/**
 * \brief       Starts SMTPD after creating the Selector
 */
int smtpd_start(Selector selector);

/****************************************************************/
/* Public function definitions                                  */
/****************************************************************/

int main(int argc, char ** argv){
    int sv_fd_4 = -1, sv_fd_6 = -1;
    Selector selector = NULL;

    TRY{
        /* Create Logger */
        THROW_IF_NOT(
            Logger_create(
                LOG_FILE                // Absolute path to the file that will hold the logs
            )
        );                              // Expected return: true

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
        THROW_IF_NOT(selector = Selector_create(free))

        /* Add both of the server sockets to the Selector */
        THROW_IF_NOT(
            Selector_add(
                selector,               // The Selector itself
                sv_fd_4,                // File descriptor to add
                SELECTOR_READ,          // Mode
                SOCK_TYPE_SERVER,       // File descriptor type
                (void *) selector       // Selector passed as data
            )
            == SELECTOR_OK              // Expected return: SELECTOR_OK
        );
        THROW_IF_NOT(
            Selector_add(
                selector,               // The Selector itself
                sv_fd_6,                // File descriptor to add
                SELECTOR_READ,          // Mode
                SOCK_TYPE_SERVER,       // File descriptor type
                (void *) selector       // Selector passed as data
            )
            == SELECTOR_OK              // Expected return: SELECTOR_OK
        );
    }
    CATCH{
        safe_close(sv_fd_4);
        safe_close(sv_fd_6);
        Selector_cleanup(selector);
        return EXIT_FAILURE;
    }

    return smtpd_start(selector);
}

/****************************************************************/
/* Private function definitions                                 */
/****************************************************************/

int smtpd_start(Selector selector){
    while (true) {

    }
}