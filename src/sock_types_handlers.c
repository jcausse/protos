/**
 * \file        sock_types_handlers.c
 * \brief       Handlers for all socket types used by the server.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#include "sock_types_handlers.h"

/***********************************************************************************************/
/* Read handler declarations                                                                   */
/***********************************************************************************************/

HandlerErrors handle_server (int fd, void * selector){
    return HANDLER_OK;
}

HandlerErrors handle_client_read (int fd, void * data){
    return HANDLER_OK;
}

HandlerErrors handle_manager_read (int fd, void * data){
    return HANDLER_OK;
}

/***********************************************************************************************/
/* Write handler declarations                                                                  */
/***********************************************************************************************/

HandlerErrors handle_client_write (int fd, void * data){
    return HANDLER_OK;
}

HandlerErrors handle_manager_write (int fd, void * data){
    return HANDLER_OK;
}