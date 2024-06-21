/**
 * \file        selector.c
 * \brief       Selector allows monitoring of multiple file descriptors at
 *              the same time, useful for non-blocking socket applications.
 * 
 * \note        HashMap library is required.
 * \note        LinkedList library is required.
 * \note        Exceptions header file is required.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
*/

#include "selector.h"

#define NO_TYPE -1
#define NO_DATA NULL

/*************************************************************************/
/* Private data structures                                               */
/*************************************************************************/

typedef struct _Selector_t {
    LinkedList      read_fds;       // List of file descriptors added for READ operations.
    fd_set          read_set;       // Set of file descriptors added for READ operations.

    LinkedList      write_fds;      // List of file descriptors added for WRITE operations.
    fd_set          write_set;      // Set of file descriptors added for READ operations.

    int             maxfd;          // Greatest file descriptor number (of both read an write sets).

    HashMap         fd_types;       // HashMap containing all file descriptor types.
    HashMap         fd_data;        // HashMap containing all file descriptor data pointers.

    SelectorDataCleanupCallback data_free_fn; // Callback used for freeing file descriptor associated data.

    bool            use_timeout;    // Indicates whether the timeout should be passed to select (2) or NULL.
    struct timeval  timeout;        // Timeout used for select (2).

    LinkedList      read_ready;     // List of fds ready for a READ operation after a Selector_select call.
    LinkedList      write_ready;    // List of fds ready for a WRITE operation after a Selector_select call.
} _Selector_t;

struct _LListArg {
    fd_set *    set;
    LinkedList  list;
};

/*************************************************************************/
/* Private functions                                                     */
/*************************************************************************/

/**
 * \brief       Compare an integer *current* with another *candidate* and save
 *              the greater value (plus 1) to *current*.
 * 
 * \param[in out] current       Current maximum value.
 * \param[in]     candidate     Candidate to maximum value.
 */
static inline void set_if_greater(int * const current, int candidate);

/**
 * \brief       Check if the received mode is valid or not.
 * 
 * \param[in] mode  Mode to check.
 * 
 * \return      Boolean value: true if invalid, false otherwise.
 */
static inline bool is_invalid_mode(uint32_t mode);

/**
 * \brief       Callback used to create a LinkedList with all file descriptors
 *              ready after a select (2) operation. This list is created from
 *              the list of all available file descriptors, and the fd_set
 *              containing all ready file descriptors.
 * 
 * \param[in] fd    The file descriptor to add to the list if is present in the fd_set.
 * \param[in] arg   The argument passed to LinkedList_foreach. It is a structure
 *                  _LListArg that contains the fd_set with all ready file descriptors,
 *                  and a reference to the target list.
 */
static void activity_list_creator(int fd, void * arg);

/**
 * \brief       Get the next file descriptor available for a READ / WRITE operation.
 *              This behaviour is determined by the LinkedList passed as a parameter (for
 *              instance, if the list is self->read_ready, this function returns the next
 *              fd that is available for reading).
 * 
 * \param[in]  self     The selector itself.
 * \param[in]  list     The LinkedList to take the next fd from.
 * \param[out] type     A pointer where to store the returned file descriptor's associated type.
 * \param[out] data     A pointer where to store the returned file descriptor's associated data.
 * 
 * \return      Returns the next file descriptor that is ready, or SELECTOR_NO_FD if there is no
 *              file descriptor available for that operation.
 */
static int _Selector_next(Selector const self, LinkedList list, int * type, void ** data);

/**
 * \brief       Callback to free memory from the *type* HashMap. It is used to call SELECTOR_FREE.
 * 
 * \param[in] ptr       Pointer to data to be free'd.
 */
static void _Selector_free_cb(void * ptr);

/**
 * \brief       Callback used to close open file descriptors.
 * 
 * \param[in] fd        File descriptor to close
 * \param[in] ignored   Ignored parameter
 */
static void _Selector_fd_close_cb(int fd, void * ignored);

/*************************************************************************/
/* HashMap callbacks                                                     */
/*************************************************************************/

static uint32_t multiplicative_hash(const uint16_t key, const size_t size);
static bool key_equals(const uint16_t key1, const uint16_t key2);

/*************************************************************************/
/* Public functions                                                      */
/*************************************************************************/

Selector Selector_create(SelectorDataCleanupCallback data_free_cb){
    return Selector_create_timeout(-1, data_free_cb);
}

Selector Selector_create_timeout(int timeout, SelectorDataCleanupCallback data_free_cb){
    if (timeout != -1 && timeout < 1){
        return NULL;
    }
    Selector self = NULL;
    TRY{
        THROW_IF((self              = SELECTOR_CALLOC(1, sizeof(_Selector_t))        ) == NULL);
        THROW_IF((self->read_fds    = LinkedList_create()                            ) == NULL);
        THROW_IF((self->write_fds   = LinkedList_create()                            ) == NULL);
        THROW_IF((self->read_ready  = LinkedList_create()                            ) == NULL);
        THROW_IF((self->write_ready = LinkedList_create()                            ) == NULL);
        THROW_IF((self->fd_types    = HashMap_create(multiplicative_hash, key_equals)) == NULL);
        THROW_IF((self->fd_data     = HashMap_create(multiplicative_hash, key_equals)) == NULL);
    }
    CATCH{
        if (self != NULL){
            LinkedList_cleanup(self->read_fds);     // NULL-safe
            LinkedList_cleanup(self->write_fds);    // NULL-safe
            LinkedList_cleanup(self->read_ready);   // NULL-safe
            LinkedList_cleanup(self->write_ready);  // NULL-safe
            HashMap_cleanup(self->fd_types, NULL);  // NULL-safe
            HashMap_cleanup(self->fd_data, NULL);   // NULL-safe
        }
        return NULL;
    }
    self->data_free_fn = data_free_cb;
    if (timeout == -1){
        self->use_timeout = false;
    }
    else{
        self->use_timeout = true;
        self->timeout.tv_sec = timeout;
    }
    self->read_ready  = NULL;
    self->write_ready = NULL;
    return self;
}

SelectorErrors Selector_add(Selector const self, 
    const int fd, 
    SelectorModes mode, 
    int type, 
    void * data
){
    /* Check if a valid Selector has been received */
    if (self == NULL){
        return SELECTOR_INVALID;
    }

    /* Check if a valid mode has been received */
    if (is_invalid_mode(mode)){
        return SELECTOR_BAD_MODE;
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

    /* Attempt to save type to dynamic memory */
    int * type_internal = SELECTOR_MALLOC(sizeof(int));
    if (type_internal == NULL){
        return SELECTOR_NO_MEMORY;
    }
    * type_internal = type;

    /* Attempt to insert the fd to the lists */
    /* 
     * Prepend (not append) the file descriptor, because LinkedList_pop is O(n),
     * while LinkedList_shift has time complexity O(1).
     */
    if (add_read && LinkedList_prepend(self->read_fds, fd) != LINKEDLIST_OK){
        SELECTOR_FREE(type_internal);
        return SELECTOR_NO_MEMORY;
    }
    if (add_write && LinkedList_prepend(self->write_fds, fd) == LINKEDLIST_OK){
        /* 
         * Remove file descriptor from the read_fds LinkedList if an error occurred when
         * adding it to the write_fds LinkedList.
         */
        if(add_read){
            LinkedList_shift(self->read_fds, NULL);  // Time complexity: O(1).
        }
        SELECTOR_FREE(type_internal);
        return SELECTOR_NO_MEMORY;
    }

    /* Attempt to insert data and type into the HashMaps */
    HashMapErrors err;
    bool on_error_remove_type = false;
    if (type >= 0){
        /*
         * Note that HashMap_put returns HASHMAP_DUPLICATED_KEY if fd is already present.
         * This error is ignored on purpose: as stated in the Selector documentation, the
         * type and data associated to the file descriptor are not modified if it has previously
         * been added to the Selector (original type and data is kept).
         */
        if((err = HashMap_put(self->fd_types, fd, (void *) type_internal)) == HASHMAP_NO_MEMORY){
            if (add_read){
                LinkedList_shift(self->read_fds, NULL);
            }
            if (add_write){
                LinkedList_shift(self->write_fds, NULL);
            }
            SELECTOR_FREE(type_internal);
            return SELECTOR_NO_MEMORY;
        }
        if (err != HASHMAP_DUPLICATED_KEY){
            on_error_remove_type = true;
        }
    }
    else{
        SELECTOR_FREE(type_internal);
    }
    if (data != NULL){
        if (HashMap_put(self->fd_data, fd, data) == HASHMAP_NO_MEMORY){
            /* 
             * Remove file descriptor from the fd_types HashMap if an error occurred when
             * adding it to the fd_data HashMap.
             */
            if (on_error_remove_type){
                HashMap_pop(self->fd_types, fd, NULL);
            }
            if (add_read){
                LinkedList_shift(self->read_fds, NULL);
            }
            if (add_write){
                LinkedList_shift(self->write_fds, NULL);
            }
            SELECTOR_FREE(type_internal);
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

SelectorErrors Selector_remove(Selector const self, 
    const int fd, 
    SelectorModes mode,
    bool free_data
){
    /* Check if a valid Selector has been received */
    if (self == NULL){
        return SELECTOR_INVALID;
    }

    /* Check if a valid mode has been received */
    if (is_invalid_mode(mode)){
        return SELECTOR_BAD_MODE;
    }

    /* Remove the file descriptor from the sets */
    bool remove_read   = (mode & SELECTOR_READ)  != 0;
    bool remove_write  = (mode & SELECTOR_WRITE) != 0;
    bool should_remove_data = remove_read && remove_write;
    if (remove_read){
        FD_CLR(fd, &(self->read_set));
        LinkedList_remove_elem(self->read_fds, fd, true);
        if (! should_remove_data){
            should_remove_data = ! FD_ISSET(fd, &(self->write_set));
        }
    }
    if (remove_write){
        FD_CLR(fd, &(self->write_set));
        LinkedList_remove_elem(self->write_fds, fd, true);
        if (! should_remove_data){
            should_remove_data = ! FD_ISSET(fd, &(self->read_set));
        }
    }

    /* 
     * Remove the data from the HashMaps, if necessary. Keep in mind that
     * HashMap_pop might return HASHMAP_KEY_NOT_FOUND. This error is ignored on purpose.
     */
    if (should_remove_data){
        /* Remove the file descriptor type */
        int  * type = NULL;
        HashMap_pop(self->fd_types, fd, (void **) &type);
        if (type != NULL){
            SELECTOR_FREE(type);
        }

        /* Remove the file descriptor data */
        void * data = NULL;
        HashMap_pop(self->fd_data,  fd, &data);
        if (type != NULL && free_data){
            self->data_free_fn(data);
        }
    }

    return SELECTOR_OK;
}

SelectorErrors Selector_select(Selector const self){
    /* Check if a valid Selector has been received */
    if (self == NULL){
        return SELECTOR_INVALID;
    }

    /* Clear the ready lists */
    LinkedList_clear(self->read_ready);
    LinkedList_clear(self->write_ready);

    /* Create a copy of the file descriptor sets and the timeout structure */
    fd_set readers, writers;
    struct timeval timeout;
    SELECTOR_MEMCPY(&readers, &(self->read_set),    sizeof(fd_set));
    SELECTOR_MEMCPY(&writers, &(self->write_set),   sizeof(fd_set));
    if (self->use_timeout){
        SELECTOR_MEMCPY(&timeout, &(self->timeout),     sizeof(struct timeval));
    }

    /* Create a struct _LListArg to hold needed arguments for LinkedList_foreach */
    struct _LListArg arg;

    /* Perform a select (2) operation */
    int activity;
    TRY{
        THROW_IF(-1 == (activity = 
            select(
                self->maxfd, 
                &readers, 
                &writers, 
                NULL, 
                self->use_timeout ? &timeout : NULL
            )
        ));
    }
    CATCH{
        return SELECTOR_SELECT_ERR;
    }

    /* Populate read_ready with all file descriptors from read_fds that are ready for reading */
    arg.set  = &readers;
    arg.list = self->read_ready;
    LinkedList_foreach(&(self->read_fds),  activity_list_creator, (void *) &arg);

    /* Populate write_ready with all file descriptors from write_fds that are ready for writing */
    arg.set  = &writers;
    arg.list = self->write_ready;
    LinkedList_foreach(&(self->write_fds), activity_list_creator, (void *) &arg);
    
    return SELECTOR_OK;
}

int Selector_read_next(Selector const self, int * type, void ** data){
    if (self == NULL){
        return SELECTOR_INVALID;
    }
    return _Selector_next(self, self->read_ready, type, data);
}

int Selector_write_next(Selector const self, int * type, void ** data){
    if (self == NULL){
        return SELECTOR_INVALID;
    }
    return _Selector_next(self, self->write_ready, type, data);
}

void Selector_cleanup(Selector self){
    if (self == NULL){
        return;
    }

    /* Close all file descriptors */
    LinkedList_foreach(&(self->read_fds),  _Selector_fd_close_cb, NULL);
    LinkedList_foreach(&(self->write_fds), _Selector_fd_close_cb, NULL);

    /* Clear LinkedLists */
    LinkedList_cleanup(self->read_fds);
    LinkedList_cleanup(self->write_fds);
    LinkedList_cleanup(self->read_ready);
    LinkedList_cleanup(self->write_ready);

    /* Clear HashMaps */
    HashMap_cleanup(self->fd_types, _Selector_free_cb);
    HashMap_cleanup(self->fd_data, self->data_free_fn);

    SELECTOR_FREE(self);
}

/*************************************************************************/
/* Private functions                                                     */
/*************************************************************************/

static inline void set_if_greater(int * const current, int candidate){
    if (candidate >= (* current)){
        * current = candidate + 1;
    }
}

static inline bool is_invalid_mode(uint32_t mode){
    return (bool)(mode & (~ SELECTOR_READ_WRITE));
}

static void activity_list_creator(int fd, void * arg){
    struct _LListArg * arg_cast = (struct _LListArg *) arg;
    fd_set * set = arg_cast->set;
    LinkedList list = arg_cast->list;

    if (FD_ISSET(fd, set)){
        LinkedList_append(list, fd);  // Ignores LINKEDLIST_NO_MEMORY
    }
}

static int _Selector_next(Selector const self, LinkedList list, int * type, void ** data){
    int fd;
    int fd_type, * fd_type_ptr;
    void * fd_data;

    /* Attempt to get the next available fd, and return SELECTOR_NO_FD if no fd is ready */
    if (LinkedList_shift(list, &fd) == LINKEDLIST_EMPTY){
        return SELECTOR_NO_FD;
    }

    /* Attempt to get the associated type */
    if (HashMap_peek(self->fd_types, fd, (void *)(&fd_type_ptr)) == HASHMAP_KEY_NOT_FOUND){
        fd_type = NO_TYPE;
    }
    else{
        fd_type = * fd_type_ptr;
    }

    /* Attempt to get the associated data */
    if (HashMap_peek(self->fd_data, fd, &fd_data) == HASHMAP_KEY_NOT_FOUND){
        fd_data = NO_DATA;
    }

    * type = fd_type;
    * data = fd_data;
    return fd;
}

static void _Selector_free_cb(void * ptr){
    SELECTOR_FREE(ptr);
}

static void _Selector_fd_close_cb(int fd, void * ignored){
    (void) ignored;     // Avoids unused parameter warnings
    close(fd);
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
