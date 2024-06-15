#include "selector.h"

#define MAX_SOCK 1023               // Maximum file descriptor number (imposed by glibc)

#define SOCK_ARRAY_EL_MAX_COUNT (MAX_SOCK + 1)      // Maximum number of fds in a fd array.
#define SOCK_ARRAY_EL_SIZE (sizeof(int))            // Size of 1 file descriptor.

/*************************************************************************/
/* Private data structures                                               */
/*************************************************************************/

typedef struct _Selector_t {
    LinkedList      read_fds;       // File descriptors added for READ operations.
    fd_set          read_set;       // Sockets added for READ operations.

    LinkedList      write_fds;      // File descriptors added for WRITE operations.
    fd_set          write_set;      // Sockets added for WRITE operations.

    int             maxfd;          // Greatest file descriptor number (of both read an write sets).

    HashMap         sock_types;     // HashMap containing all socket types.
    HashMap         sock_data;      // HashMap containing all socket data pointers.

    bool            use_timeout;    // Indicates whether the timeout should be passed to select(3) or NULL.
    struct timeval  timeout;        // Timeout used for select (2).
} _Selector_t;

/*************************************************************************/
/* Private functions                                                     */
/*************************************************************************/

/**
 * \brief       Compare an integer *current* with another *candidate* and save
 *              the greater value to *current*.
 * 
 * \param[in out] current       Current maximum value.
 * \param[in]     candidate     Candidate to maximum value.
 */
static inline void set_if_greater(int * const current, int candidate);

/*************************************************************************/
/* HashMap callbacks                                                     */
/*************************************************************************/

static uint32_t multiplicative_hash(const uint16_t key, const size_t size);
static bool key_equals(const uint16_t key1, const uint16_t key2);

/*************************************************************************/
/* Public functions                                                      */
/*************************************************************************/

Selector Selector_create(){
    return Selector_create_timeout(-1);
}

Selector Selector_create_timeout(int timeout){
    if (timeout != -1 && timeout < 1){
        return NULL;
    }
    Selector self = NULL;
    TRY{
        THROW_IF((self = SELECTOR_CALLOC(1, sizeof(_Selector_t))) == NULL);
        THROW_IF((self->read_fds = LinkedList_create()) == NULL);
        THROW_IF((self->write_fds = LinkedList_create()) == NULL);
        THROW_IF((self->sock_types = HashMap_create(multiplicative_hash, key_equals)) == NULL);
        THROW_IF((self->sock_data = HashMap_create(multiplicative_hash, key_equals)) == NULL);
    }
    CATCH{
        if (self != NULL){
            LinkedList_cleanup(self->read_fds);         // NULL-safe
            LinkedList_cleanup(self->write_fds);        // NULL-safe
            HashMap_cleanup(self->sock_types, NULL);    // NULL-safe
            HashMap_cleanup(self->sock_data, NULL);     // NULL-safe
        }
        return NULL;
    }
    if (timeout == -1){
        self->use_timeout = false;
    }
    else{
        self->use_timeout = true;
        self->timeout.tv_sec = timeout;
    }
    return self;
}

SelectorErrors Selector_add(Selector const self, 
    const int fd, 
    SelectorModes mode, 
    const int type, 
    const void * const data
){
    /* Check if an valid Selector has been received */
    if (self == NULL){
        return SELECTOR_INVALID;
    }

    /* Check if there is any operation to perform */
    bool add_read   = (bool) (mode & SELECTOR_READ);
    bool add_write  = (bool) (mode & SELECTOR_WRITE);
    if (add_read && FD_ISSET(fd, &(self->read_set))){
        add_read = false;
    }
    if (add_write && FD_ISSET(fd, &(self->write_set))){
        add_write = false;
    }

    /* If the file descriptor is already added for the specified mode(s), return */
    if (! (add_read || add_write)){
        return SELECTOR_OK;
    }

    /* Attempt to insert the fd to the lists */
    {   
        bool on_error_remove = false;
        TRY{
            if (add_read){
                /* 
                 * Prepend (not append) the file descriptor, because LinkedList_pop is O(n),
                 * while LinkedList_shift has time complexity O(1).
                 */
                THROW_IF_NOT(LinkedList_prepend(self->read_fds, fd) == LINKEDLIST_OK);
                on_error_remove = true;
            }
            if (add_write){
                THROW_IF_NOT(LinkedList_prepend(self->write_fds, fd) == LINKEDLIST_OK);
            }
        }
        CATCH{
            /* 
             * Remove file descriptor from the read_fds LinkedList if an error occurred when
             * adding it to the write_fds LinkedList.
             */
            if (on_error_remove){
                LinkedList_shift(self->read_fds, NULL);  // Time complexity: O(1).
            }
            return SELECTOR_NO_MEMORY;
        }
    }

    /* Attempt to insert data and type into the HashMaps */
    {
        HashMapErrors err;
        bool on_error_remove_type = false;
        TRY{
            
            if (type > 0){
                /*
                 * Note that HashMap_put returns HASHMAP_DUPLICATED_KEY if fd is already present.
                 * This error is ignored on purpose: as stated in the Selector documentation, the
                 * type and data associated to the file descriptor are not modified if it has previously
                 * been added to the Selector (original type and data is kept).
                 */
                err = HashMap_put(self->sock_types, fd, type);
                THROW_IF(err == HASHMAP_NO_MEMORY);
                if (err != HASHMAP_DUPLICATED_KEY){
                    on_error_remove_type = true;
                }
            }
            if (data != NULL){
                err = HashMap_put(self->sock_data, fd, data);
                THROW_IF(err == HASHMAP_NO_MEMORY);
            }
        }
        CATCH{
            /* 
             * Remove file descriptor from the sock_types HashMap if an error occurred when
             * adding it to the sock_data HashMap.
             */
            if (on_error_remove_type){
                HashMap_pop(self->sock_types, fd, NULL);
            }
            if (add_read){
                LinkedList_shift(self->read_fds, NULL);
            }
            if (add_write){
                LinkedList_shift(self->write_fds, NULL);
            }
            return SELECTOR_NO_MEMORY;
        }
    }

    /* Set the file descriptor to the corresponding set(s) */
    if (add_read){
        FD_SET(fd, &(self->read_set));
    }
    if (add_write){
        FD_SET(fd, &(self->write_set));
    }

    /* Set the greater file descriptor number yet */
    set_if_greater(&(self->maxfd), fd);

    return SELECTOR_OK;
}

SelectorErrors Selector_remove(Selector const self, const int fd, SelectorModes mode){
    // TODO
    return SELECTOR_OK;
}

SelectorErrors Selector_select(Selector const self){
    // TODO
    return SELECTOR_OK;
}

bool Selector_read_has_next(Selector const self){
    // TODO
    return false;
}

SelectorErrors Selector_read_next(Selector const self, int * fd, int * type, void ** data){
    // TODO
    return SELECTOR_OK;
}

bool Selector_write_has_next(Selector const self){
    // TODO
    return false;
}

SelectorErrors Selector_write_next(Selector const self, int * fd, int * type, void ** data){
    // TODO
    return SELECTOR_OK;
}

void Selector_cleanup(Selector self /* TODO Add HashMap cleanup callback */ ){
    // TODO
}

/*************************************************************************/
/* Private functions                                                     */
/*************************************************************************/

static inline void set_if_greater(int * const current, int candidate){
    if (candidate > (* current)){
        * current = candidate;
    }
}

/*************************************************************************/
/* HashMap callbacks                                                     */
/*************************************************************************/

static unsigned int num_digits(unsigned int number) {
    if (number == 0){
        return 1;
    }
    int digits = 0;
    while (number > 0){
        number /= 10;
        digits++;
    }
    return digits;
}

static uint32_t multiplicative_hash(const uint16_t key, const size_t size) {
    const unsigned long long A = 2654435769U; // Knuth's multiplicative constant. See https://gist.github.com/badboy/6267743.
    unsigned long long hash = key * A;
    return (hash >> (32 - num_digits(size))) % size;
}

static bool key_equals(const uint16_t key1, const uint16_t key2){
    return key1 == key2;
}
