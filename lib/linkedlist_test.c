#include "linkedlist.h"
#include <assert.h>
#include <stdio.h>

void additional_tests(void);

void increment(int value, void * arg){
    * ((int *) arg) += value;
}

void modify(int value, void * arg) {
    LinkedList * list_ptr = (LinkedList *) arg;
    LinkedList_append(* list_ptr, value);
}

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

    for (int i = 0; i < 500000; ++i) {
        err = LinkedList_shift(list, &elem);
        assert(err == LINKEDLIST_OK);
        assert(elem == i + 500000);
    }

    assert(LinkedList_size(list) == 0);

    // Test 5: LinkedList_remove_elem
    err = LinkedList_append(list, 30);
    assert(err == LINKEDLIST_OK);
    err = LinkedList_append(list, 40);
    assert(err == LINKEDLIST_OK);
    err = LinkedList_append(list, 30);
    assert(err == LINKEDLIST_OK);
    err = LinkedList_append(list, 30);
    assert(err == LINKEDLIST_OK);
    err = LinkedList_append(list, 50);
    assert(err == LINKEDLIST_OK);
    err = LinkedList_append(list, 30);
    assert(err == LINKEDLIST_OK);
    assert(LinkedList_size(list) == 6);

    // Remove first occurrence of 30
    err = LinkedList_remove_elem(list, 30, true);
    assert(err == LINKEDLIST_OK);
    assert(LinkedList_size(list) == 5);

    // Ensure only the first 30 was removed
    err = LinkedList_get(list, &elem, 0);
    assert(err == LINKEDLIST_OK);
    assert(elem == 40);
    err = LinkedList_get(list, &elem, 1);
    assert(err == LINKEDLIST_OK);
    assert(elem == 30);
    err = LinkedList_get(list, &elem, 2);
    assert(err == LINKEDLIST_OK);
    assert(elem == 30);
    err = LinkedList_get(list, &elem, 3);
    assert(err == LINKEDLIST_OK);
    assert(elem == 50);
    err = LinkedList_get(list, &elem, 4);
    assert(err == LINKEDLIST_OK);
    assert(elem == 30);

    // Remove all occurrences of 30
    err = LinkedList_remove_elem(list, 30, false);
    assert(err == LINKEDLIST_OK);
    assert(LinkedList_size(list) == 2);

    // Ensure no 30 is left
    err = LinkedList_get(list, &elem, 0);
    assert(err == LINKEDLIST_OK);
    assert(elem == 40);
    err = LinkedList_get(list, &elem, 1);
    assert(err == LINKEDLIST_OK);
    assert(elem == 50);

    // Test 6: LinkedList_foreach
    int sum = 0;
    err = LinkedList_foreach(&list, increment, &sum);
    assert(err == LINKEDLIST_OK);
    assert(sum == (40 + 50));

    assert(LinkedList_pop(list, NULL) == LINKEDLIST_OK);
    assert(LinkedList_pop(list, NULL) == LINKEDLIST_OK);
    assert(LinkedList_size(list) == 0);
    assert(LinkedList_pop(list, NULL) == LINKEDLIST_EMPTY);

    for (int i = 1; i <= 5000; i++){
        assert(LinkedList_insert(list, i, i -1) == LINKEDLIST_OK);  // Like append
    }

    sum = 0;
    err = LinkedList_foreach(&list, increment, &sum);
    assert(err == LINKEDLIST_OK);
    assert(sum == (5000 * 5001) / 2); // Sum of integers from 1 to 50000

    // Test LinkedList_foreach with modification attempt
    err = LinkedList_foreach(&list, modify, &list);
    assert(err == LINKEDLIST_INVALID);
    assert(list == NULL);

    // Recreate list for further testing
    list = LinkedList_create();
    assert(list != NULL);

    for (int i = 0; i < 10; ++i) {
        err = LinkedList_append(list, i);
        assert(err == LINKEDLIST_OK);
    }

    sum = 0;
    err = LinkedList_foreach(&list, increment, &sum);
    assert(err == LINKEDLIST_OK);
    assert(sum == 45); // Sum of first 10 integers (0 through 9)

    // Cleanup
    LinkedList_cleanup(list);

    puts("LinkedList: All tests passed!");
    additional_tests();
}

// Additional edge case tests

void additional_tests(void) {
    // Test 1: Sequential Insertions and Deletions
    LinkedList list = LinkedList_create();
    assert(list != NULL);

    // Insert 100 elements at the head and 100 at the tail
    for (int i = 0; i < 100; ++i) {
        assert(LinkedList_prepend(list, i) == LINKEDLIST_OK);
        assert(LinkedList_append(list, i) == LINKEDLIST_OK);
    }

    // Remove 100 elements from the head and 100 from the tail
    int elem;
    size_t size;
    for (int i = 0; i < 100; ++i) {
        assert(LinkedList_remove(list, &elem, 0) == LINKEDLIST_OK);
        size = LinkedList_size(list);
        assert(size == (size_t)(200 - 2 * i - 1));
        assert(LinkedList_remove(list, &elem, size - 1) == LINKEDLIST_OK);
    }
    assert(LinkedList_size(list) == 0);

    // Clean up
    LinkedList_cleanup(list);

    // Test 2: Removing all elements one by one
    list = LinkedList_create();
    assert(list != NULL);

    // Insert 10 elements
    for (int i = 0; i < 10; ++i) {
        assert(LinkedList_append(list, i) == LINKEDLIST_OK);
    }

    // Remove all elements one by one
    for (int i = 0; i < 10; ++i) {
        assert(LinkedList_shift(list, &elem) == LINKEDLIST_OK);
    }

    assert(LinkedList_size(list) == 0);

    // Clean up
    LinkedList_cleanup(list);

    // Test 3: Removing elements from an empty list
    list = LinkedList_create();
    assert(list != NULL);

    // Try to remove an element from the empty list
    assert(LinkedList_shift(list, &elem) == LINKEDLIST_EMPTY);

    // Clean up
    LinkedList_cleanup(list);

    // Test 4: Stress Testing
    list = LinkedList_create();
    assert(list != NULL);

    // Insert 100,000 elements
    for (int i = 0; i < 100000; ++i) {
        assert(LinkedList_append(list, i) == LINKEDLIST_OK);
    }

    // Remove 100,000 elements
    for (int i = 0; i < 100000; ++i) {
        assert(LinkedList_shift(list, &elem) == LINKEDLIST_OK);
    }

    assert(LinkedList_size(list) == 0);

    // Clean up
    LinkedList_cleanup(list);

    puts("LinkedList: All additional tests passed!");
}

