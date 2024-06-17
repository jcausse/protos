/**
 * \file        sockets.h
 * \brief       Various socket utilities.
 * 
 * \note        Exceptions header file is required.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#ifndef __SOCKETS_H_2hf9742bc23__
#define __SOCKETS_H_2hf9742bc23__

#include <sys/socket.h>     // socket(), AF_INET, AF_INET6, SOCK_STREAM
#include <netinet/in.h>     // IPPROTO_TCP, INADDR_ANY, in6addr_any
#include <arpa/inet.h>      // htons(), inet_pton()
#include <stdint.h>         // uint16_t
#include <string.h>         // memset()
#include <stdbool.h>        // bool
#include <unistd.h>         // close()
#include "exceptions.h"     // TRY, THROW_IF, CATCH

/*************************************************************************/

#define SOCK_OK     0
#define SOCK_FAIL   -1

/*************************************************************************/

/**
 * \brief       Create and connect an active socket.
 * 
 * \param[in] ip            Remote IP Address to connect to.
 * \param[in] port          Remote port to connect to.
 * \param[in] ipv6          Use IPv6 instead of IPv4.
 * \param[in] keep_alive    Enable TCP Keep Alive for this socket.
 * \param[in] rst           Send RST instead of FIN when closing the connection.
 * 
 * \return      Socket file descriptor on success, SOCK_FAIL on failure.
 */
int tcp_connect(
    const char * restrict ip,
    uint16_t port,
    bool ipv6,
    bool keep_alive,
    bool rst
);

/**
 * \brief       Create and bind passive sockets for both IPv4 and IPv6.
 * 
 * \param[in] port          Port to listen on.
 * \param[in] backlog       Backlog size for the listen queue.
 * \param[out] ipv4_sockfd  IPv4 socket file descriptor.
 * \param[out] ipv6_sockfd  IPv6 socket file descriptor.
 * 
 * \return      true on success, false on failure.
 */
bool tcp_serve(
    uint16_t port,
    unsigned int backlog,
    int * const ipv4_sockfd,
    int * const ipv6_sockfd
);

#endif // __SOCKETS_H_2hf9742bc23__
