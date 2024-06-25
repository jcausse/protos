/**
 * \file        sock_types_handlers.h
 * \brief       Handlers for all socket types used by the server.
 *
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#ifndef __SOCK_TYPES_H__
#define __SOCK_TYPES_H__

#include <stddef.h>     // NULL
#include <unistd.h>     // close(), write()
#include <sys/socket.h> // accept(), recv()
#include <arpa/inet.h>  // sockaddr_in, sockaddr_in6
#include <sys/types.h>
#include <errno.h>      // errno, EWOULDBLOCK, EAGAIN, EINTR
#include "lib/logger.h"
#include "utils/selector.h"
#include "messages.h"
#include "utils/stats.h"

/***********************************************************************************************/
/* Custom data type definitions                                                                */
/***********************************************************************************************/

/**
 * \enum        HandlerErrors: errors returned by socket READ / WRITE handlers.
 */
typedef enum{
    HANDLER_OK      =  0,       // No error occurred.
    HANDLER_NO_MEM  = -1,       // No memory available.
    HANDLER_NO_OP   = -2,       // Perform no operation (like HANDLER_OK, but ignoring some kind of error).
} HandlerErrors;

/**
 * \typedef     SockReadHandler: Typedef of function that handles reads from a socket.
 *
 *              Parameters are:
 *              1. The socket file descriptor (of type int).
 *              2. The data associated to that file descriptor (of type void *).
 *
 *              These functions return `HandlerErrors`.
 */
typedef HandlerErrors (* SockReadHandler) (int, void *);

/**
 * \typedef     SockWriteHandler: Typedef of function that handles writes to a socket.
 *
 *              Parameters are:
 *              1. The socket file descriptor (of type int).
 *              2. The data associated to that file descriptor (of type void *).
 *
 *              These functions return `HandlerErrors`.
 */
typedef HandlerErrors (* SockWriteHandler) (int, void *);

/*  XX(SOCKET_TYPE,             READ_HANDLER,                   WRITE_HANDLER               ) */
#define SOCK_TYPES_AND_HANDLERS(XX)                                                           \
    XX(SOCK_TYPE_SERVER4,       handle_server4,                 NULL                        ) \
    XX(SOCK_TYPE_SERVER6,       handle_server6,                 NULL                        ) \
    XX(SOCK_TYPE_CLIENT,        handle_client_read,             handle_client_write         ) \
    XX(SOCK_TYPE_MANAGER,       handle_manager_read,            handle_manager_write        )

/**
 * \enum        SockTypes: socket types used in the Selector.
 */
typedef enum {
    #define XX(sock_type_numeric, sock_read_handler, sock_write_handler) sock_type_numeric,
    SOCK_TYPES_AND_HANDLERS(XX)
    #undef XX
    SOCK_TYPE_QTY
} SockTypes;

/***********************************************************************************************/
/* Read handler declarations                                                                   */
/***********************************************************************************************/

/**
 * \brief       Handle new connection requests from an IPv4 server socket (a passive socket listening
 *              for new connections).
 *
 * \details     Accepts those new connections and adds them to the Selector to perform a WRITE
 *              operation (as it is part of SMTP's RFC).
 *              This function attempts to accept all pending connections until `accept (2)` returns
 *              EWOULDBLOCK or EAGAIN. For this reason, it is crucial that the server socket is set as non
 *              blocking using `fcntl (2)` and option `O_NONBLOCK`.
 *
 * \param[in] fd        The server socket file descriptor to which perform an accept (2).
 * \param[in] _         Unused parameter.
 *
 * \return      Returns any of the following error codes:
 *              - HANDLER_OK
 */
HandlerErrors handle_server4            (int fd, void * _);

/**
 * \brief       Handle new connection requests from n IPv6 server socket (a passive socket listening
 *              for new connections).
 *
 * \details     Accepts those new connections and adds them to the Selector to perform a WRITE
 *              operation (as it is part of SMTP's RFC).
 *              This function attempts to accept all pending connections until `accept (2)` returns
 *              EWOULDBLOCK or EAGAIN. For this reason, it is crucial that the server socket is set as non
 *              blocking using `fcntl (2)` and option `O_NONBLOCK`.
 *
 * \param[in] fd        The server socket file descriptor to which perform an accept (2).
 * \param[in] _         Unused parameter.
 *
 * \return      Returns any of the following error codes:
 *              - HANDLER_OK
 */
HandlerErrors handle_server6            (int fd, void * _);

/**
 * \brief       Handle client incoming data (a READ operation to the client's socket).
 *
 * \param[in] fd        The socket connected the client to read data from.
 * \param[in] data      The data associated to that client.
 *
 * \return      Returns any of the following error codes:
 *              - HANDLER_OK
 */
HandlerErrors handle_client_read        (int fd, void * data);

/**
 * \brief       Handle a new command from the manager.
 *
 * \param[in] fd        The management socket (UDP) to read a command from.
 * \param[in] data      \todo que data va aca??
 *
 * \return      Returns any of the following error codes:
 *              - HANDLER_OK
 */
HandlerErrors handle_manager_read       (int fd, void * data);

/***********************************************************************************************/
/* Write handler declarations                                                                  */
/***********************************************************************************************/

/**
 * \brief       Handle client outgoing data (a WRITE operation to the client's socket).
 *
 * \param[in] fd        The socket connected the client to write data to.
 * \param[in] data      The data associated to that client.
 *
 * \return      Returns any of the following error codes:
 *              - HANDLER_OK
 */
HandlerErrors handle_client_write       (int fd, void * data);

/**
 * \brief       Handle a command response to the manager.
 *
 * \param[in] fd        The management socket (UDP) to which the response will be sent.
 * \param[in] data      \todo que data va aca??
 *
 * \return      Returns any of the following error codes:
 *              - HANDLER_OK
 */
HandlerErrors handle_manager_write      (int fd, void * data);

#endif // __SOCK_TYPES_H__
