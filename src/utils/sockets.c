/**
 * \file        sockets.c
 * \brief       Various socket utilities.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#include "sockets.h"

#define THROW_ON_ERR(expr) THROW_IF((expr) < 0)

int tcp_connect(const char * restrict ip, uint16_t port, bool ipv6, bool keep_alive, bool rst){
    sa_family_t sock_family = ipv6 ? AF_INET6 : AF_INET;
    int sockfd = -1;

    TRY{
        /* Create socket */
        THROW_ON_ERR(sockfd = socket(sock_family, SOCK_STREAM, IPPROTO_TCP));

        /* If keep_alive is set, enable TCP Keep Alive */
        if (keep_alive){
            int optval = 1;  // 1 means option enabled
            THROW_ON_ERR(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)));
        }

        /* If rst is set, set the Linger option */
        if (rst){
            struct linger optval = {1, 0};  // 1 means option enabled, 0 means TCP will discard unsent data and send RST
            THROW_ON_ERR(setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &optval, sizeof(optval)));
        }

        /* Create and initialize struct sockaddr */
        if (ipv6){
            struct sockaddr_in6 sv_addr6;
            memset(&sv_addr6, 0, sizeof(sv_addr6));
            sv_addr6.sin6_family = sock_family;
            sv_addr6.sin6_port = htons(port);
            THROW_IF(inet_pton(sock_family, ip, &sv_addr6.sin6_addr) <= 0);

            /* Connect */
            THROW_ON_ERR(connect(sockfd, (struct sockaddr *)&sv_addr6, sizeof(sv_addr6)));
        }
        else{
            struct sockaddr_in sv_addr;
            memset(&sv_addr, 0, sizeof(sv_addr));
            sv_addr.sin_family = sock_family;
            sv_addr.sin_port = htons(port);
            THROW_IF(inet_pton(sock_family, ip, &sv_addr.sin_addr) <= 0);

            /* Connect */
            THROW_ON_ERR(connect(sockfd, (struct sockaddr *)&sv_addr, sizeof(sv_addr)));
        }
    }

    CATCH{
        if (sockfd != -1){
            close(sockfd);
        }
        return SOCK_FAIL;
    }

    return sockfd;
}

bool tcp_serve(uint16_t port, unsigned int backlog, int * const ipv4_sockfd, int * const ipv6_sockfd){
    int ipv4_fd = -1, ipv6_fd = -1, flags;
    const int optval = 1;                   // Value used for socket option SO_REUSEADDR
    struct linger linger_optval = {1, 0};   // Value used for socket option SO_LINGER

    TRY{
        /******************** IP v4 ********************/

        /* Create IPv4 socket */
        THROW_ON_ERR(ipv4_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

        /* Set IPv4 socket options */
        THROW_ON_ERR(setsockopt(ipv4_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)));
        THROW_ON_ERR(setsockopt(ipv4_fd, SOL_SOCKET, SO_LINGER, &linger_optval, sizeof(linger_optval)));

        /* Bind IPv4 socket */
        struct sockaddr_in addr4;
        memset(&addr4, 0, sizeof(addr4));
        addr4.sin_family = AF_INET;
        addr4.sin_addr.s_addr = INADDR_ANY;
        addr4.sin_port = htons(port);
        THROW_ON_ERR(bind(ipv4_fd, (struct sockaddr *)&addr4, sizeof(addr4)));

        /* Listen on IPv4 socket */
        THROW_ON_ERR(listen(ipv4_fd, backlog));

        /* Set IPv4 socket to non-blocking mode */
        THROW_ON_ERR(flags = fcntl(ipv4_fd, F_GETFL, 0));
        THROW_ON_ERR(fcntl(ipv4_fd, F_SETFL, flags | O_NONBLOCK));

        /******************** IP v6 ********************/

        /* Create IPv6 socket */
        THROW_ON_ERR(ipv6_fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP));

        /* Set IPv6 socket options */
        THROW_ON_ERR(setsockopt(ipv6_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)));
        THROW_ON_ERR(setsockopt(ipv6_fd, SOL_SOCKET, SO_LINGER, &linger_optval, sizeof(linger_optval)));

        /* Disable dual stack to avoid conflict with IPv4 socket */
        THROW_ON_ERR(setsockopt(ipv6_fd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval)));

        /* Bind IPv6 socket */
        struct sockaddr_in6 addr6;
        memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr = in6addr_any;
        addr6.sin6_port = htons(port);
        THROW_ON_ERR(bind(ipv6_fd, (struct sockaddr *)&addr6, sizeof(addr6)));

        /* Set IPv6 socket to non-blocking mode */
        THROW_ON_ERR(flags = fcntl(ipv6_fd, F_GETFL, 0));
        THROW_ON_ERR(fcntl(ipv6_fd, F_SETFL, flags | O_NONBLOCK));

        /* Listen on IPv6 socket */
        THROW_ON_ERR(listen(ipv6_fd, backlog));
    }

    CATCH{
        if (ipv4_fd != -1) {
            close(ipv4_fd);
        } 
        if (ipv6_fd != -1) {
            close(ipv6_fd);
        }
        return false;
    }

    /* Assign socket file descriptors to output parameters and return */
    *ipv4_sockfd = ipv4_fd;
    *ipv6_sockfd = ipv6_fd;
    return true;
}

bool udp_serve(uint16_t port, int * sockfd){
    int fd;

    /* Create socket */
    if ((fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        return false;
    }

    /* Allow dual stack (IPv4 and IPv6) */
    int opt = 0;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) < 0) {
        close(fd);
        return false;
    }

    /* Create and setup address structure */
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any;
    addr.sin6_port = htons(port);

    /* Bind the socket to the specified port */
    if (bind(fd, (const struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(fd);
        return false;
    }

    * sockfd = fd;
    return true;
}

void safe_close(int fd){
    errno = 0;
    if (fd < 0){
        return;
    }
    int ret;
    do {
        ret = close(fd);
    }
    while (ret != 0 && errno == EINTR);
}

bool get_client_addr(int fd, char ** ip, uint16_t * port) {
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);

    if (getpeername(fd, (struct sockaddr *) &addr, &addr_len) == -1){
        return false;
    }

    /* IPv4 sockets */
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in * addr_in = (struct sockaddr_in *) &addr;
        if (inet_ntop(AF_INET, &(addr_in->sin_addr), * ip, INET_ADDRSTRLEN) == NULL){
            return false;
        }
        * port = ntohs(addr_in->sin_port);
    }

    /* IPv6 sockets */
    else if (addr.ss_family == AF_INET6){
        struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) &addr;
        if (inet_ntop(AF_INET6, &(addr_in6->sin6_addr), * ip, INET6_ADDRSTRLEN) == NULL){
            return false;
        }
        * port = ntohs(addr_in6->sin6_port);
    } 
    
    /* Unsupported address family */
    else {
        return false;
    }

    return true;
}

