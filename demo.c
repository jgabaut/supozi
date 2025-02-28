#define SPZ_IMPLEMENTATION
#include "supozi.h"

// Define the test functions
TEST(void, test_addition) {
    if (1 + 1 != 2) {
        printf("[  FAILED  ] foo test_addition!\n");
    } else {
        printf("[  PASSED  ] bar test_addition!\n");
    }
}

TEST(void, test_subtraction) {
    if (3 - 2 != 1) {
        printf("[  FAILED  ] foo test_subtraction!\n");
    } else {
        printf("[  PASSED  ] bar test_subtraction!\n");
    }
}

TEST(bool, test_foo) {
    printf("FOO\n");
    return false;
}

// Use a macro to automatically register all tests and define main()
#define TEST_LIST \
    REGISTER_TEST(test_addition); \
    REGISTER_TEST(test_subtraction); \
    REGISTER_TEST(test_foo);

REGISTER_ALL_TESTS();  // This will automatically define the main function and register the tests
