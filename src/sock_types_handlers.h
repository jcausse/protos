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

/**
 * \enum        HandlerErrors: errors returned by socket READ / WRITE handlers.
 * 
 * \todo Agregar definiciones de errores.
 */
typedef enum{
    HANDLER_OK      = 0,        // No error occurred
} HandlerErrors;

/***********************************************************************************************/
/* Read handler declarations                                                                   */
/***********************************************************************************************/

/**
 * \brief       Handle new connection requests from a server socket (a passive socket listening
 *              for new connections).
 * 
 * \details     Accepts those new connections and adds them to the Selector to perform a WRITE
 *              operation (as it is part of SMTP's RFC).
 * 
 * \param[in] fd        The server socket file descriptor to which perform an accept (2).
 * \param[in] selector  The Selector to add the connection to after accepting it.
 * 
 * \return      Returns any of the following error codes:
 *              - HANDLER_OK
 */
HandlerErrors handle_server          (int fd, void * selector);

/**
 * \brief       Handle client incoming data (a READ operation to the client's socket).
 * 
 * \param[in] fd        The socket connected the client to read data from.
 * \param[in] data      The data associated to that client.
 * 
 * \return      Returns any of the following error codes:
 *              - HANDLER_OK
 */
HandlerErrors handle_client_read     (int fd, void * data);

/**
 * \brief       Handle a new command from the manager.
 * 
 * \param[in] fd        The management socket (UDP) to read a command from.
 * \param[in] data      \todo que data va aca??
 * 
 * \return      Returns any of the following error codes:
 *              - HANDLER_OK
 */
HandlerErrors handle_manager_read    (int fd, void * data);

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
HandlerErrors handle_client_write    (int fd, void * data);

/**
 * \brief       Handle a command response to the manager.
 * 
 * \param[in] fd        The management socket (UDP) to which the response will be sent.
 * \param[in] data      \todo que data va aca??
 * 
 * \return      Returns any of the following error codes:
 *              - HANDLER_OK
 */
HandlerErrors handle_manager_write   (int fd, void * data);

/***********************************************************************************************/
/* Custom data type definitions                                                                */
/***********************************************************************************************/

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

/***********************************************************************************************/
/* Table containing all socket type identifier and handlers                                    */
/***********************************************************************************************/

/*  XX(SOCKET_TYPE,             READ_HANDLER,                   WRITE_HANDLER               ) */

#define SOCK_TYPES_AND_HANDLERS(XX)                                                           \
    XX(SOCK_TYPE_SERVER,        handle_server,                  NULL                        ) \
    XX(SOCK_TYPE_CLIENT,        handle_client_read,             handle_client_write         ) \
    XX(SOCK_TYPE_MANAGER,       handle_manager_read,            handle_manager_write        )

/***********************************************************************************************/
/* Autogenerated constant enumeration and function pointer arrays used by the server           */
/***********************************************************************************************/

/**
 * \enum        SockTypes: socket types used in the Selector.
 */
typedef enum {
    #define XX(sock_type_numeric, sock_read_handler, sock_write_handler) sock_type_numeric,
    SOCK_TYPES_AND_HANDLERS(XX)
    #undef XX
    SOCK_TYPE_QTY
} SockTypes;

#endif // __SOCK_TYPES_H__
