#include "selector.h"

#define MAX_SOCK 1023               // Maximum file descriptor number (imposed by glibc)

#define SOCK_ARRAY_EL_MAX_COUNT (MAX_SOCK + 1)      // Maximum number of fds in a fd array.
#define SOCK_ARRAY_EL_SIZE (sizeof(int))            // Size of 1 file descriptor.

typedef struct _Selector_t {
    int *           read_fds;       // Array of file descriptors added for READ operations.
    unsigned int    read_count;     // Number of sockets added for READ operations.
    fd_set          read_set;       // Sockets added for READ operations.

    int *           write_fds;      // Array of file descriptors added for WRITE operations.
    unsigned int    write_count;    // Number of sockets added for WRITE operations.
    fd_set          write_set;      // Sockets added for WRITE operations.

    int             maxfd;          // Greatest file descriptor number (of both read an write sets).

    HashMap         sock_types;     // HashMap containing all socket types.
    HashMap         sock_data;      // HashMap containing all socket data pointers.

    struct timeval  timeout;        // Timeout used for select (2).
} _Selector_t;

/*************************************************************************/

Selector Selector_create(){
    return Selector_create_timeout(-1);
}

Selector Selector_create_timeout(int timeout){
    Selector self = SELECTOR_CALLOC(1, sizeof(_Selector_t));
    if (self == NULL){
        return NULL;
    }

    // TODO
}