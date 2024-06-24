/**
 * \file        linkedlist.h
 * \brief       Create and manage a Linked List of integers.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <stddef.h>
#include <stdbool.h>

/*************************************************************************/
/*                              CUSTOMIZABLE                             */
/*************************************************************************/

#include <stdlib.h>

/* Memory allocation function equivalent to malloc (3) or a malloc (3) wrapper. May not initialize the allocated zone. */
#define LINKEDLIST_MALLOC(size) malloc((size))

/* Memory freeing function equivalent to free (3) or a free (3) wrapper. */
#define LINKEDLIST_FREE(ptr) free((ptr))

/*************************************************************************/

/**
 * \typedef     LinkedList main Abstract Data Type.
*/
typedef struct _LinkedList_t * LinkedList;

/**
 * \enum        Errors.
*/
typedef enum {
    LINKEDLIST_OK                 =  0,   // No error.
    LINKEDLIST_NO_MEMORY          = -1,   // Not enough memory (LINKEDLIST_MALLOC error).
    LINKEDLIST_BAD_INDEX          = -2,   // Invalid index when inserting or removing an element.
    LINKEDLIST_EMPTY              = -3,   // Attempted to remove an element from an empty LinkedList.
    LINKEDLIST_INVALID            = -4,   // Invalid LinkedList (*self* parameter may be NULL).
    LINKEDLIST_BAD_FUN            = -5    // NULL *fun* parameter for LinkedList_foreach.
} LinkedListErrors;

/*************************************************************************/

/**
 * \brief       Create an empty LinkedList.
 * 
 * \return      A LinkedList on success, NULL on error.
*/
LinkedList LinkedList_create();

/**
 * \brief       Insert an element at the beginning of the LinkedList.
 * 
 * \param[in] self      The LinkedList itself.
 * \param[in] elem      The element to be added at the beginning.
 * 
 * \return      LINKEDLIST_OK on success, LINKEDLIST_NO_MEMORY on memory
 *              allocation error. LINKEDLIST_INVALID if self is NULL.
*/
LinkedListErrors LinkedList_prepend(LinkedList const self, int elem);

/**
 * \brief       Insert an element at the end of the LinkedList.
 * 
 * \param[in] self      The LinkedList itself.
 * \param[in] elem      The element to be added at the end.
 * 
 * \return      LINKEDLIST_OK on success, LINKEDLIST_NO_MEMORY on memory
 *              allocation error. LINKEDLIST_INVALID if self is NULL.
*/
LinkedListErrors LinkedList_append(LinkedList const self, int elem);

/**
 * \brief       Insert an element at a given position in the LinkedList.
 * 
 * \param[in] self      The LinkedList itself.
 * \param[in] elem      The element to be added.
 * \param[in] index     The index at which the element will be inserted.
 *                      Note that index should be in [0, SIZE], where SIZE is
 *                      the number of elements in the LinkedList.
 * 
 * \return      LINKEDLIST_OK on success, LINKEDLIST_BAD_INDEX if no such index
 *              exists in the LinkedList, or LINKEDLIST_NO_MEMORY on memory
 *              allocation error. LINKEDLIST_INVALID if self is NULL.
*/
LinkedListErrors LinkedList_insert(LinkedList const self, int elem, unsigned int index);

/**
 * \brief       Remove the first element from the LinkedList.
 * 
 * \param[in] self      The LinkedList itself.
 * \param[out] elem     The removed element. If an error occurs, the target value
 *                      is left as is. NULL safe.
 * 
 * \return      LINKEDLIST_OK on success, or LINKEDLIST_EMPTY if no element was to
 *              be removed. LINKEDLIST_INVALID if self is NULL.
*/
LinkedListErrors LinkedList_shift(LinkedList const self, int * elem);

/**
 * \brief       Remove the last element from the LinkedList.
 * 
 * \param[in] self      The LinkedList itself.
 * \param[out] elem     The removed element. If an error occurs, the target value
 *                      is left as is. NULL safe.
 * 
 * \return      LINKEDLIST_OK on success, or LINKEDLIST_EMPTY if no element was to
 *              be removed. LINKEDLIST_INVALID if self is NULL.
*/
LinkedListErrors LinkedList_pop(LinkedList const self, int * elem);

/**
 * \brief       Remove an element at a given position in the LinkedList.
 * 
 * \param[in] self      The LinkedList itself.
 * \param[out] elem     The removed element. If an error occurs, the target value
 *                      is left as is. NULL safe.
 * \param[in] index     The index of the element to be removed.
 *                      Note that index should be in [0, SIZE - 1], where SIZE is
 *                      the number of elements in the LinkedList.
 * 
 * \return      LINKEDLIST_OK on success, LINKEDLIST_BAD_INDEX if no such index
 *              exists in the LinkedList, LINKEDLIST_EMPTY if no element was to
 *              be removed (the LinkedList contains no elements). LINKEDLIST_INVALID 
 *              if self is NULL.
*/
LinkedListErrors LinkedList_remove(LinkedList const self, int * elem, unsigned int index);

/**
 * \brief       Remove an element passed as parameter from the LinkedList.
 * 
 * \param[in] self          The LinkedList itself.
 * \param[in] elem          The element to be removed.
 * \param[in] first_only    If true, only removes the first occurrence of the element.
 *                          If false, traverse the entire list and remove every occurrence
 *                          of the element.
 * 
 * \return      LINKEDLIST_OK on success. LINKEDLIST_EMPTY if no element was to be removed 
 *              (the LinkedList contains no elements).  LINKEDLIST_INVALID if self is NULL.
 */
LinkedListErrors LinkedList_remove_elem(LinkedList const self, int elem, bool first_only);

/**
 * \brief       Get the first element from the LinkedList without removing it.
 * 
 * \param[in] self      The LinkedList itself.
 * \param[out] elem     The desired element. If an error occurs, the target value
 *                      is left as is. NULL safe.
 * 
 * \return      LINKEDLIST_OK on success, or LINKEDLIST_EMPTY if the LinkedList 
 *              contains no elements. LINKEDLIST_INVALID if self is NULL.
*/
LinkedListErrors LinkedList_get_first(LinkedList const self, int * elem);

/**
 * \brief       Get the last element from the LinkedList without removing it.
 * 
 * \param[in] self      The LinkedList itself.
 * \param[out] elem     The desired element. If an error occurs, the target value
 *                      is left as is. NULL safe.
 * 
 * \return      LINKEDLIST_OK on success, or LINKEDLIST_EMPTY if the LinkedList 
 *              contains no elements. LINKEDLIST_INVALID if self is NULL.
*/
LinkedListErrors LinkedList_get_last(LinkedList const self, int * elem);

/**
 * \brief       Get an element at a given position in the LinkedList without removing it.
 * 
 * \param[in] self      The LinkedList itself.
 * \param[out] elem     The desired element. If an error occurs, the target value
 *                      is left as is. NULL safe.
 * \param[in] index     The index of the element to be retrieved.
 *                      Note that index should be in [0, SIZE - 1], where SIZE is
 *                      the number of elements in the LinkedList.
 * 
 * \return      LINKEDLIST_OK on success, LINKEDLIST_BAD_INDEX if no such index
 *              exists in the LinkedList, LINKEDLIST_EMPTY if the LinkedList contains 
 *              no elements. LINKEDLIST_INVALID if self is NULL.
*/
LinkedListErrors LinkedList_get(LinkedList const self, int * elem, unsigned int index);

/**
 * \brief       Execute a function *fun* for each element in the LinkedList.
 * 
 * \details     The *fun* callback MUST NOT modify the LinkedList, since modifications 
 *              to the LinkedList while iterating may cause its internal state to 
 *              become inconsistent. 
 *              Any attempt to modify the LinkedList while iterating will cause the 
 *              LinkedList to self destroy, *self* to be nulled, and LinkedList_foreach
 *              to return LINKEDLIST_INVALID.
 *              The following methods shall NOT be invoked while iterating:
 *              - LinkedList_prepend
 *              - LinkedList_append
 *              - LinkedList_insert
 *              - LinkedList_shift
 *              - LinkedList_pop
 *              - LinkedList_remove
 *              - LinkedList_remove_elem
 * 
 * \param[in out] self  The LinkedList itself (will be assigned NULL if modified during
 *                      iteration).
 * \param[in] fun       Function to be called. This function will be called for each
 *                      element in the LinkedList (and therefore will be called
 *                      *LinkedList_size(self)* times). The function shall not return
 *                      any value, and shall receive two arguments:
 *                      - The element itself (an integer).
 *                      - An optional argument (of type void *).
 * \param[in] arg       Option argument to pass to *fun* each time it is called.
 * 
 * \return      LINKEDLIST_OK on success. LINKEDLIST_EMPTY if the LinkedList contains 
 *              no elements. LINKEDLIST_INVALID if self is NULL or if the LinkedList is
 *              modified during iteration. LINKEDLIST_BAD_FUN if *fun* is NULL;
 */
LinkedListErrors LinkedList_foreach(LinkedList * self, 
    void (* fun)(int, void *), 
    void * arg
);

/**
 * \brief       Get the amount of elements contained in the LinkedList.
 * 
 * \param[in] self      The LinkedList itself.
 * 
 * \return      The size of the LinkedList. 0 if self is NULL.
*/
size_t LinkedList_size(LinkedList const self);

/**
 * \brief       Remove every element in the LinkedList without destroying it, so it can
 *              be reused. This is done more efficiently than just doing *LinkedList_pop*
 *              operations until it is empty.
 *              The LinkedList is left with the same state as a newly created one.
 * 
 * \param[in] self      The LinkedList itself.
 * 
 * \return      LINKEDLIST_OK on success. LINKEDLIST_INVALID if self is NULL.
 */
LinkedListErrors LinkedList_clear(LinkedList const self);

/**
 * \brief       Destroy the LinkedList and free all memory.
 * 
 * \param[in] self      The LinkedList itself.
*/
void LinkedList_cleanup(LinkedList self);

#endif // __LINKEDLIST_H__