#include "hashmap.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define TEST_COUNT 65534

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

// Test helper function to generate a string based on an integer
static char *generate_string(int num) {
    size_t size = snprintf(NULL, 0, "string%d", num) + 1; // +1 for null terminator
    char *str = (char*)malloc(size);
    if (str) {
        snprintf(str, size, "string%d", num);
    }
    return str;
}

// Custom cleanup callback to free allocated strings
static void cleanup_callback(HashMapValue val) {
    free(val);
}

int main(void) {
    HashMap map = HashMap_create(multiplicative_hash, key_equals);
    HashMapValue val;

    assert(HashMap_put(map, 1, generate_string(1)) == HASHMAP_OK);
    assert(HashMap_put(map, 0, generate_string(2)) == HASHMAP_OK);
    assert(HashMap_size(map) == 2);
    assert(HashMap_contains(map, 1) == true);
    assert(HashMap_contains(map, 0) == true);

    assert(HashMap_put(map, 1, "prueba") == HASHMAP_DUPLICATED_KEY);
    assert(HashMap_size(map) == 2);
    assert(HashMap_contains(map, 1) == true);

    assert(HashMap_put(map, 15321, generate_string(4)) == HASHMAP_OK);
    assert(HashMap_size(map) == 3);
    assert(HashMap_contains(map, 1) == true);
    assert(HashMap_contains(map, 0) == true);
    assert(HashMap_contains(map, 15321) == true);

    assert(HashMap_peek(map, 0, &val) == HASHMAP_OK);
    assert(strcmp((char*)val, "string2") == 0);
    assert(HashMap_size(map) == 3);
    assert(HashMap_contains(map, 0) == true);

    assert(HashMap_pop(map, 1, &val) == HASHMAP_OK);
    assert(strcmp((char*)val, "string1") == 0);
    free(val); // Manually free popped value
    assert(HashMap_size(map) == 2);
    assert(HashMap_contains(map, 1) == false);

    val = (HashMapValue)0x1234;
    assert(HashMap_peek(map, 5, &val) == HASHMAP_KEY_NOT_FOUND);
    assert(val == (HashMapValue)0x1234);
    assert(HashMap_size(map) == 2);

    val = (HashMapValue)0x1234;
    assert(HashMap_pop(map, 14322, &val) == HASHMAP_KEY_NOT_FOUND);
    assert(val == (HashMapValue)0x1234);
    assert(HashMap_size(map) == 2);

    assert(HashMap_pop(map, 0, &val) == HASHMAP_OK);
    assert(strcmp((char*)val, "string2") == 0);
    free(val); // Manually free popped value
    assert(HashMap_size(map) == 1);
    assert(HashMap_contains(map, 0) == false);

    assert(HashMap_pop(map, 15321, &val) == HASHMAP_OK);
    assert(strcmp((char*)val, "string4") == 0);
    free(val); // Manually free popped value
    assert(HashMap_size(map) == 0);
    assert(HashMap_contains(map, 15321) == false);

    // Test large number of elements
    for (int i = 0; i < TEST_COUNT; i++) {
        assert(HashMap_put(map, i, generate_string(i)) == HASHMAP_OK);
    }
    assert(HashMap_size(map) == TEST_COUNT);

    // Check that all elements are in the map
    char buff[32];
    for (int i = 0; i < TEST_COUNT; i++) {
        assert(HashMap_contains(map, i) == true);
        assert(HashMap_peek(map, i, &val) == HASHMAP_OK);
        sprintf(buff, "string%d", i);
        assert(strcmp((char*)val, buff) == 0);
    }

    // Remove some elements
    for (int i = 0; i < TEST_COUNT / 2; i++) {
        assert(HashMap_pop(map, i, &val) == HASHMAP_OK);
        sprintf(buff, "string%d", i);
        assert(strcmp((char*)val, buff) == 0);
        free(val); // Manually free popped value
    }
    assert(HashMap_size(map) == TEST_COUNT / 2);

    // Check that removed elements are no longer in the map
    for (int i = 0; i < TEST_COUNT / 2; i++) {
        assert(HashMap_contains(map, i) == false);
    }

    // Check remaining elements
    for (int i = TEST_COUNT / 2; i < TEST_COUNT; i++) {
        assert(HashMap_contains(map, i) == true);
        assert(HashMap_peek(map, i, &val) == HASHMAP_OK);
        sprintf(buff, "string%d", i);
        assert(strcmp((char*)val, buff) == 0);
    }

    // Test for hash collisions
    assert(HashMap_put(map, 1, generate_string(1)) == HASHMAP_OK); // Previously removed
    assert(HashMap_put(map, 1, "string1") == HASHMAP_DUPLICATED_KEY);
    for (int i = TEST_COUNT / 2; i < TEST_COUNT; i++) {
        assert(HashMap_put(map, i, "prueba") == HASHMAP_DUPLICATED_KEY);
    }

    // Cleanup and memory check
    HashMap_cleanup(map, cleanup_callback);
    // Do not check size after cleanup, since map is invalid after cleanup

    puts("HashMap: All tests passed!");
}
