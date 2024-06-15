/**
 * \file        selector.h
 * \details     Selector allows monitoring of multiple file descriptors at
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

/* Memory allocation function equivalent to malloc (3) or a malloc (3) wrapper. May not initialize the allocated zone. */
#define SELECTOR_MALLOC(size) malloc((size))

/* Memory allocation function equivalent to calloc (3) or a calloc (3) wrapper. Must initialize the allocated zone to 0. */
#define SELECTOR_CALLOC(qty, el_size) calloc((qty), (el_size))

/* Memory freeing function equivalent to free (3) or a free (3) wrapper. */
#define SELECTOR_FREE(ptr) free((ptr))

/*************************************************************************/

/**
 * \typedef     Selector main Abstract Data Type.
*/
typedef struct _Selector_t * Selector;

/**
 * \enum        File descriptor modes (read, write, or read/write)
*/
typedef enum {
    SELECTOR_READ               = (1 << 0),
    SELECTOR_WRITE              = (1 << 1),
    SELECTOR_READ_WRITE         = SELECTOR_READ | SELECTOR_WRITE
} SelectorModes;

/**
 * \enum        Selector Errors.
*/
typedef enum {
    SELECTOR_OK                 =  0,   // No error.
    SELECTOR_NO_MEMORY          = -1,   // Not enough memory.
    SELECTOR_BAD_MODE           = -2,   // Invalid mode provided.
    SELECTOR_INVALID            = -3,   // Selector state is not valid or self is NULL.
    SELECTOR_FD_NOT_PRESENT     = -4,   // Attempted to remove a file descriptor that
                                        // is not present in the Selector.
} SelectorErrors;

/*************************************************************************/

/**
 * \brief       Creates a new Selector with no timeout (*Selector_select* will block until
 *              any file descriptor becomes ready).
 * \details     Calling this function is equivalent to *Selector_create_timeout(-1)*.
 * 
 * \return      A new Selector on success, NULL on failure (memory not available).
*/
Selector Selector_create();

/**
 * \brief       Creates a new Selector with a custom timeout.
 * 
 * \param[in] timeout   Amount of seconds *Selector_select* will wait for any file descriptor
 *                      to become ready for any type of operation (read / write). -1 means no
 *                      timeout, so *Selector_select* will wait indefinitely for at least 1 
 *                      file descriptor to become ready (blocking function call).
 *                      This parameter can either be set to -1 to indicate no timeout, or to a
 *                      number greater or equal to 1.
 * 
 * \return      A new Selector on success, NULL on failure (memory not available or invalid timeout).
*/
Selector Selector_create_timeout(int timeout);

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
 * \brief       Remove a file descriptor from the Selector.
 * 
 * \param[in]   self    The Selector itself, returned by Selector_create.
 * \param[in]   fd      File descriptor.
 * \param[in]   mode    Mode (read, write, read/write).
 * 
 * \return      Can return the following error codes (see Selector Errors enumeration):
 *              1. SELECTOR_OK
 *              2. SELECTOR_INVALID
 *              3. SELECTOR_BAD_MODE
*/
SelectorErrors Selector_remove(Selector const self, const int fd, SelectorModes mode);

/**
 * TODO
*/
SelectorErrors Selector_select(Selector const self);

/**
 * TODO
*/
bool Selector_read_has_next(Selector const self);

/**
 * TODO
*/
SelectorErrors Selector_read_next(Selector const self, int * fd, int * type, void ** data);

/**
 * TODO
*/
bool Selector_write_has_next(Selector const self);

/**
 * TODO
*/
SelectorErrors Selector_write_next(Selector const self, int * fd, int * type, void ** data);

/**
 * TODO
*/
void Selector_cleanup(Selector self /* TODO Add HashMap cleanup callback */ );

#endif // __SELECTOR_H__