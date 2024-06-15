/**
 * \file        linkedlist.c
 * \details     Create and manage a Linked List of integers.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#include "linkedlist.h"

typedef struct _LinkedList_Node_t {
    int value;
    struct _LinkedList_Node_t * next;
} _LinkedList_Node_t;

typedef struct _LinkedList_t {
    struct _LinkedList_Node_t * head;
    struct _LinkedList_Node_t * tail; 
    size_t size;
    bool was_modified;
} _LinkedList_t;

/*************************************************************************/
/* Private functions                                                     */
/*************************************************************************/
/**
 * \brief   Remove an element from the LinkedList
 * 
 * \param[in] self      The LinkedList itself.
 * \param[out] elem     Pointer to an int where to save the removed value.
 *                      NULL-safe (if elem is NULL, value does not get saved).
 * \param[in] current   Node where the element was found.
 * \param[in] last      The previous Node to the one where the element was found.
 */
static void _LinkedList_remove(LinkedList const self, 
    int * elem, _LinkedList_Node_t * current, _LinkedList_Node_t * last
);

/*************************************************************************/
/* Public functions                                                      */
/*************************************************************************/

LinkedList LinkedList_create() {
    LinkedList self = LINKEDLIST_MALLOC(sizeof(_LinkedList_t));
    if (self != NULL) {
        self->head = NULL;
        self->tail = NULL;
        self->size = 0;
        self->was_modified = false;
    }
    return self;
}

LinkedListErrors LinkedList_prepend(LinkedList const self, int elem) {
    if (self == NULL) {
        return LINKEDLIST_INVALID;
    }

    _LinkedList_Node_t* new_node = LINKEDLIST_MALLOC(sizeof(_LinkedList_Node_t));
    if (new_node == NULL) {
        return LINKEDLIST_NO_MEMORY;
    }

    new_node->value = elem;
    new_node->next = self->head;
    self->head = new_node;

    if (self->size == 0) {
        self->tail = new_node;
    }

    self->size++;
    self->was_modified = true;
    return LINKEDLIST_OK;
}

LinkedListErrors LinkedList_append(LinkedList const self, int elem) {
    if (self == NULL) {
        return LINKEDLIST_INVALID;
    }

    _LinkedList_Node_t* new_node = LINKEDLIST_MALLOC(sizeof(_LinkedList_Node_t));
    if (new_node == NULL) {
        return LINKEDLIST_NO_MEMORY;
    }

    new_node->value = elem;
    new_node->next = NULL;

    if (self->size == 0) {
        self->head = new_node;
        self->tail = new_node;
    } 
    else {
        self->tail->next = new_node;
        self->tail = new_node;
    }

    self->size++;
    self->was_modified = true;
    return LINKEDLIST_OK;
}

LinkedListErrors LinkedList_insert(LinkedList const self, int elem, unsigned int index) {
    if (self == NULL) {
        return LINKEDLIST_INVALID;
    }
    if (index > self->size) {
        return LINKEDLIST_BAD_INDEX;
    }

    if (index == 0) {
        return LinkedList_prepend(self, elem);
    } 
    else if (index == self->size) {
        return LinkedList_append(self, elem);
    }

    _LinkedList_Node_t* new_node = LINKEDLIST_MALLOC(sizeof(_LinkedList_Node_t));
    if (new_node == NULL) {
        return LINKEDLIST_NO_MEMORY;
    }

    new_node->value = elem;

    _LinkedList_Node_t * current = self->head;
    for (unsigned int i = 0; i < index - 1; i++) {
        current = current->next;
    }

    new_node->next = current->next;
    current->next = new_node;

    self->size++;
    self->was_modified = true;
    return LINKEDLIST_OK;
}

LinkedListErrors LinkedList_shift(LinkedList const self, int * elem) {
    return LinkedList_remove(self, elem, 0);
}

LinkedListErrors LinkedList_pop(LinkedList const self, int * elem) {
    if (self == NULL) {
        return LINKEDLIST_INVALID;
    }
    return LinkedList_remove(self, elem, self->size - 1);
}

LinkedListErrors LinkedList_remove(LinkedList const self, int * elem, unsigned int index) {
    if (self == NULL) {
        return LINKEDLIST_INVALID;
    }
    if (self->size == 0) {
        return LINKEDLIST_EMPTY;
    }
    if (index >= self->size) {
        return LINKEDLIST_BAD_INDEX;
    }

    _LinkedList_Node_t * current = self->head, * last = NULL;
    for (unsigned int i = 0; i < index; i++) {
        last = current;
        current = current->next;
    }

    _LinkedList_remove(self, elem, current, last);

    return LINKEDLIST_OK;
}

LinkedListErrors LinkedList_remove_elem(LinkedList const self, int elem, bool first_only){
    if (self == NULL) {
        return LINKEDLIST_INVALID;
    }
    if (self->size == 0) {
        return LINKEDLIST_EMPTY;
    }

    _LinkedList_Node_t * current = self->head, * last = NULL, * next = NULL;
    while (current != NULL){
        next = current->next;
        if (current->value == elem){
            _LinkedList_remove(self, NULL, current, last);
            if (first_only){
                return LINKEDLIST_OK;
            }
        }
        else{
            last = current;
        }
        current = next;
    }

    return LINKEDLIST_OK;
}

LinkedListErrors LinkedList_get_first(LinkedList const self, int * elem) {
    return LinkedList_get(self, elem, 0);
}

LinkedListErrors LinkedList_get_last(LinkedList const self, int * elem) {
    if (self == NULL) {
        return LINKEDLIST_INVALID;
    }
    if (self->size == 0) {
        return LINKEDLIST_EMPTY;
    }

    if (elem != NULL){
        * elem = self->tail->value;
    }
    return LINKEDLIST_OK;
}

LinkedListErrors LinkedList_get(LinkedList const self, int * elem, unsigned int index) {
    if (self == NULL) {
        return LINKEDLIST_INVALID;
    }
    if (index >= self->size) {
        return LINKEDLIST_BAD_INDEX;
    }

    _LinkedList_Node_t* current = self->head;
    for (unsigned int i = 0; i < index; i++) {
        current = current->next;
    }

    if (elem != NULL) {
        * elem = current->value;
    }
    return LINKEDLIST_OK;
}

LinkedListErrors LinkedList_foreach(LinkedList * self, 
    void (* fun)(int, void *), 
    void * arg
){
    if (self == NULL){
        return LINKEDLIST_INVALID;
    }
    LinkedList list = * self;
    if (list == NULL){
        return LINKEDLIST_INVALID;
    }
    if (list->size == 0){
        return LINKEDLIST_EMPTY;
    }
    if (fun == NULL){
        return LINKEDLIST_BAD_FUN;
    }

    list->was_modified = false;

    _LinkedList_Node_t * current = list->head;
    for (unsigned int i = 0; i < list->size; i++){
        if (self == NULL || * self != list){
            return LINKEDLIST_INVALID;
        }
        if (list->was_modified){
            LinkedList_cleanup(list);
            * self = NULL;
            return LINKEDLIST_INVALID;
        }
        fun(current->value, arg);
        current = current->next;
    }

    return LINKEDLIST_OK;
}

size_t LinkedList_size(LinkedList const self) {
    return self == NULL ? 0 : self->size;
}

void LinkedList_cleanup(LinkedList self){
    if (self != NULL && self->head != NULL){
        _LinkedList_Node_t * current = self->head, * next;
        while (current != NULL){
            next = current->next;
            LINKEDLIST_FREE(current);
            current = next;
        }
    }
    LINKEDLIST_FREE(self);
}

/*************************************************************************/
/* Private functions                                                     */
/*************************************************************************/

static void _LinkedList_remove(LinkedList const self, 
    int * elem, _LinkedList_Node_t * current, _LinkedList_Node_t * last
){
    if (elem != NULL) {
        * elem = current->value;
    }
    
    if (last != NULL) {
        last->next = current->next;
    }
    if (current == self->head) {
        self->head = current->next;
    }
    if (current == self->tail) {
        self->tail = last;
    }

    LINKEDLIST_FREE(current);
    self->was_modified = true;
    self->size--;
}
