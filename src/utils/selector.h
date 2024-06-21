/**
 * \file        selector.h
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

#ifndef __SELECTOR_H__
#define __SELECTOR_H__

#include <stdbool.h>
#include <sys/select.h>
#include "hashmap.h"
#include "linkedlist.h"
#include "exceptions.h"

/*************************************************************************/
/*                              CUSTOMIZABLE                             */
/*************************************************************************/

#include <stdlib.h>
#include <string.h>

/* Memory allocation function equivalent to malloc (3) or a malloc (3) wrapper. May not initialize the allocated zone. */
#define SELECTOR_MALLOC(size) malloc((size))

/* Memory allocation function equivalent to calloc (3) or a calloc (3) wrapper. Must initialize the allocated zone to 0. */
#define SELECTOR_CALLOC(qty, el_size) calloc((qty), (el_size))

/* Memory freeing function equivalent to free (3) or a free (3) wrapper. */
#define SELECTOR_FREE(ptr) free((ptr))

/* Memory copying function equivalent to memcpy (3) or a memcpy (3) wrapper. */
#define SELECTOR_MEMCPY(dest, src, n) memcpy((dest), (src), (n))

/*************************************************************************/

/**
 * \typedef     Selector main Abstract Data Type.
*/
typedef struct _Selector_t * Selector;

/**
 * \typedef     Callback used to free file descriptor data when a file descriptor is removed from the Selector, or when performing
 *              a cleanup. Must receive exactly one parameter to which the data will be passed (as void *), and return
 *              nothing.
 */
typedef void (* SelectorDataCleanupCallback) (void *);

/**
 * \enum        File descriptor modes (read, write, or read/write)
*/
typedef enum {
    SELECTOR_READ               = (1 << 0),
    SELECTOR_WRITE              = (1 << 1),
    SELECTOR_READ_WRITE         = SELECTOR_READ | SELECTOR_WRITE
} SelectorModes;

/**
 * \enum        Selector Errors. All constants MUST be less than zero (except SELECTOR_OK).
*/
typedef enum {
    SELECTOR_OK         =  0,   // No error.
    SELECTOR_NO_MEMORY  = -1,   // Not enough memory (SELECTOR_MALLOC or SELECTOR_CALLOC returned NULL).
    SELECTOR_BAD_MODE   = -2,   // Invalid mode provided. Provided *mode* must be listed in *SelectorModes*.
    SELECTOR_INVALID    = -3,   // Selector state is not valid or self is NULL.
    SELECTOR_SELECT_ERR = -4,   // select (2) call returned -1. *errno* is left unmodified.
    SELECTOR_NO_FD      = -5    // No file descriptor available for READ or WRITE operation. This error is
                                // returned by Selector_read_next or Selector_write_next when called to get
                                // the next fd available for its operation, but no fd is available yet.
} SelectorErrors;

/*************************************************************************/

/**
 * \brief       Creates a new Selector with no timeout (*Selector_select* will block until
 *              any file descriptor becomes ready).
 * \details     Calling this function is equivalent to *Selector_create_timeout(-1, data_free_cb)*.
 * 
 * \param[in] data_free_cb  Callback used to free file descriptor data when a file descriptor is
 *                          removed from the Selector, or when performing a cleanup.
 * 
 * \return      A new Selector on success, NULL on failure (memory not available or NULL *data_free_cb*).
*/
Selector Selector_create(SelectorDataCleanupCallback data_free_cb);

/**
 * \brief       Creates a new Selector with a custom timeout.
 * 
 * \param[in] timeout   Amount of seconds *Selector_select* will wait for any file descriptor
 *                      to become ready for any type of operation (read / write). -1 means no
 *                      timeout, so *Selector_select* will wait indefinitely for at least 1 
 *                      file descriptor to become ready (blocking function call).
 *                      This parameter can either be set to -1 to indicate no timeout, or to a
 *                      number greater or equal to 1.
 * \param[in] data_free_cb  Callback used to free file descriptor data when a file descriptor is
 *                          removed from the Selector, or when performing a cleanup.
 * 
 * \return      A new Selector on success, NULL on failure (memory not available, invalid timeout
 *              or NULL *data_free_cb*).
*/
Selector Selector_create_timeout(int timeout, SelectorDataCleanupCallback data_free_cb);

/**
 * \brief       Adds a new file descriptor to the Selector. If the file descriptor has
 *              already been registered for any operation, its mode is updated (it is added for
 *              any operations it is not currently subscribed for), but its *type* and its
 *              *data* are NOT modified.
 * 
 * \details     This function also allows for additional information to be attached the
 *              file descriptor: an optional "type", and an optional "data".
 *              -   The "type" is intended to hold an integer >= 0 that could help later
 *                  determine the appropiate handler function for the file descriptor.
 *                  Not attached if "type" is a negative integer.
 *              -   The "data" allows for arbitrary information of any type (as it
 *                  is a void pointer) to be associated to the file descriptor.
 *                  Not attached if "data" is NULL.
 *              Note that the provided "type" and the provided "data" are not used nor
 *              processed internally by the Selector. These parameters are ment to be
 *              retrieved along with the file descriptor when calling *Selector_read_next*
 *              or *Selector_write_next*;
 * 
 * \param[in]   self    The Selector itself, returned by Selector_create.
 * \param[in]   fd      File descriptor.
 * \param[in]   mode    Mode (read, write, read/write).
 * \param[in]   type    Additional type associated to the file descriptor (see details).
 * \param[in]   data    Additional data associated to the file descriptor (see details).
 * 
 * \return      Can return the following error codes (see Selector Errors enumeration):
 *              1. SELECTOR_OK
 *              2. SELECTOR_INVALID
 *              3. SELECTOR_NO_MEMORY
 *              4. SELECTOR_BAD_MODE
 */
SelectorErrors Selector_add(Selector const self, 
    const int fd, 
    SelectorModes mode, 
    int type, 
    void * data
);

/**
 * \brief       Desubscribe file descriptor *fd* for *mode* operation. If after removing
 *              the file descriptor for the selected operation it does not remain
 *              subscribed for any other operation, its *data* will be free'd if the
 *              *free_data* parameter is true.
 * 
 * \param[in]   self        The Selector itself, returned by Selector_create.
 * \param[in]   fd          File descriptor.
 * \param[in]   mode        Mode (read, write, read/write).
 * \param[in]   free_data   Indicates whether to free associated data or not.
 * 
 * \return      Can return the following error codes (see Selector Errors enumeration):
 *              1. SELECTOR_OK
 *              2. SELECTOR_INVALID
 *              3. SELECTOR_BAD_MODE
*/
SelectorErrors Selector_remove(Selector const self, 
    const int fd, 
    SelectorModes mode, 
    bool free_data
);

/**
 * \brief       Perform a select (2) operation on previously added file descriptors.
 * 
 * \details     After the call to this function returns, *Selector_read_has_next* and
 *              *Selector_write_has_next* must be used to check if there are file
 *              descriptors available to perform any operation. If both return false, it
 *              means that the timeout specified when creating the Selector has been 
 *              reached.
 *              This operation restarts the file descriptor iterators set by any previous
 *              *Selector_select* operations, so make sure *Selector_read_has_next* and
 *              *Selector_write_has_next* return *false* before performing a new
 *              *Selector_select* operation.
 * 
 * \param[in]   self        The Selector itself, returned by Selector_create.
 * 
 * \return      Can return the following error codes (see Selector Errors enumeration):
 *              1. SELECTOR_OK
 *              2. SELECTOR_INVALID
 *              3. SELECTOR_NO_MEMORY
 *              4. SELECTOR_SELECT_ERR
*/
SelectorErrors Selector_select(Selector const self);

/**
 * \brief       Get the next file descriptor available for reading.
 * 
 * \param[in]  self         The Selector itself, returned by Selector_create.
 * \param[out] type         The *type* associated to the file descriptor when it was
 *                          added by *Selector_add*. This parameter is set to -1 if
 *                          there is no associated type.
 * \param[out] data         The *data* associated to the file descriptor when it was
 *                          added by *Selector_add*. This parameter is set to NULL if
 *                          there is no associated type.
 * 
 * \return      Returns:
 *              1. A file descriptor available for a READ operation.
 *              2. SELECTOR_NO_FD if no fd is available for reading.
 *              3. SELECTOR_INVALID if *self* is NULL.
 */
int Selector_read_next(Selector const self, int * type, void ** data);

/**
 * \brief       Get the next file descriptor available for writing.
 * 
 * \param[in]  self         The Selector itself, returned by Selector_create.
 * \param[out] type         The *type* associated to the file descriptor when it was
 *                          added by *Selector_add*. This parameter is set to -1 if
 *                          there is no associated type.
 * \param[out] data         The *data* associated to the file descriptor when it was
 *                          added by *Selector_add*. This parameter is set to NULL if
 *                          there is no associated type.
 * 
 * \return      Returns:
 *              1. A file descriptor available for a WRITE operation.
 *              2. SELECTOR_NO_FD if no fd is available for writing.
 *              3. SELECTOR_INVALID if *self* is NULL.
 */
int Selector_write_next(Selector const self, int * type, void ** data);

/**
 * \brief       Cleanup the Selector structures and release all memory allocated for
 *              associated data.
 * 
 * \param[in] self          The Selector itself, returned by Selector_create.
 * 
 * \todo        Deberia hacer close de los fds???
*/
void Selector_cleanup(Selector self);

#endif // __SELECTOR_H__