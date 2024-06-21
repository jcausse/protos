/**
 * \file        sock_types_handlers.c
 * \brief       Handlers for all socket types used by the server.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#define __SOCK_TYPES_HANDLERS_C__

#include "sock_types_handlers.h"

/***********************************************************************************************/
/* Read handler declarations                                                                   */
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
/* Write handler declarations                                                                  */
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

#undef __SOCK_TYPES_HANDLERS_C__
