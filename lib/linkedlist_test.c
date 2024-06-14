#include "linkedlist.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    LinkedList list = LinkedList_create();
    assert(list != NULL);

    int elem;
    LinkedListErrors err;

    // Test 1: Basic operations
    err = LinkedList_append(list, 10);
    assert(err == LINKEDLIST_OK);
    assert(LinkedList_size(list) == 1);

    err = LinkedList_prepend(list, 5);
    assert(err == LINKEDLIST_OK);
    assert(LinkedList_size(list) == 2);

    err = LinkedList_insert(list, 15, 1);
    assert(err == LINKEDLIST_OK);
    assert(LinkedList_size(list) == 3);

    err = LinkedList_get_first(list, &elem);
    assert(err == LINKEDLIST_OK);
    assert(elem == 5);
    assert(LinkedList_size(list) == 3);

    err = LinkedList_get_last(list, &elem);
    assert(err == LINKEDLIST_OK);
    assert(elem == 10);
    assert(LinkedList_size(list) == 3);

    err = LinkedList_get(list, &elem, 1);
    assert(err == LINKEDLIST_OK);
    assert(elem == 15);
    assert(LinkedList_size(list) == 3);

    err = LinkedList_shift(list, &elem);
    assert(err == LINKEDLIST_OK);
    assert(elem == 5);
    assert(LinkedList_size(list) == 2);

    err = LinkedList_pop(list, &elem);
    assert(err == LINKEDLIST_OK);
    assert(elem == 10);
    assert(LinkedList_size(list) == 1);

    // Test 2: Edge cases
    err = LinkedList_append(list, 20);
    assert(err == LINKEDLIST_OK);
    assert(LinkedList_size(list) == 2);

    err = LinkedList_remove(list, NULL, 1);
    assert(err == LINKEDLIST_OK);
    assert(LinkedList_size(list) == 1);

    err = LinkedList_remove(list, &elem, 0);
    assert(err == LINKEDLIST_OK);
    assert(elem == 15);
    assert(LinkedList_size(list) == 0);

    err = LinkedList_remove(list, &elem, 0);
    assert(err == LINKEDLIST_EMPTY);
    assert(elem == 15);
    assert(LinkedList_size(list) == 0);

    // Test 3: Error handling
    err = LinkedList_shift(NULL, &elem);
    assert(err == LINKEDLIST_INVALID);
    
    err = LinkedList_pop(NULL, &elem);
    assert(err == LINKEDLIST_INVALID);

    err = LinkedList_insert(list, 25, 1);
    assert(err == LINKEDLIST_BAD_INDEX);

    err = LinkedList_remove(list, &elem, 0);
    assert(err == LINKEDLIST_EMPTY);

    err = LinkedList_remove(list, &elem, 25);
    assert(err == LINKEDLIST_EMPTY);

    err = LinkedList_get_first(NULL, &elem);
    assert(err == LINKEDLIST_INVALID);

    err = LinkedList_get_last(NULL, &elem);
    assert(err == LINKEDLIST_INVALID);

    err = LinkedList_get(NULL, &elem, 0);
    assert(err == LINKEDLIST_INVALID);

    // Test 4: Memory management
    for (int i = 0; i < 1000000; ++i) {
        err = LinkedList_append(list, i);
        assert(err == LINKEDLIST_OK);
    }

    for (int i = 0; i < 500000; ++i) {
        err = LinkedList_shift(list, &elem);
        assert(err == LINKEDLIST_OK);
        assert(elem == i);
    }

    assert(LinkedList_size(list) == 500000);

    // Cleanup
    LinkedList_cleanup(list);

    puts("LinkedList: All tests passed!");
}

