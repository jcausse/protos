/**
 * \file        sock_types_handlers.c
 * \brief       Handlers for all socket types used by the server.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#include "sock_types_handlers.h"

/***********************************************************************************************/
/* Read / Write handler pointer arrays                                                         */
/***********************************************************************************************/

/**
 *              Read handlers for each socket type
 */
SockReadHandler read_handlers[] = {
    #define XX(sock_type_numeric, sock_read_handler, sock_write_handler) sock_read_handler,
    SOCK_TYPES_AND_HANDLERS(XX)
    #undef XX
    NULL
};

/**
 *              Write handlers for each socket type
 */
SockWriteHandler write_handlers[] = {
    #define XX(sock_type_numeric, sock_read_handler, sock_write_handler) sock_write_handler,
    SOCK_TYPES_AND_HANDLERS(XX)
    #undef XX
    NULL
};

/***********************************************************************************************/
/* Read handler definitions                                                                    */
/***********************************************************************************************/

/**
 * \todo
 */
HandlerErrors handle_server (int fd, void * selector){
    (void) fd;
    (void) selector;
    return HANDLER_OK;
}

/**
 * \todo
 */
HandlerErrors handle_client_read (int fd, void * data){
    (void) fd;
    (void) data;
    return HANDLER_OK;
}

/**
 * \todo
 */
HandlerErrors handle_manager_read (int fd, void * data){
    (void) fd;
    (void) data;
    return HANDLER_OK;
}

/***********************************************************************************************/
/* Write handler definitions                                                                   */
/***********************************************************************************************/

/**
 * \todo
 */
HandlerErrors handle_client_write (int fd, void * data){
    (void) fd;
    (void) data;
    return HANDLER_OK;
}

/**
 * \todo
 */
HandlerErrors handle_manager_write (int fd, void * data){
    (void) fd;
    (void) data;
    return HANDLER_OK;
}
