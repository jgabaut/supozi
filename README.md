# supozi

A basic test framework in C.

## Table of Contents

+ [Basic example](#basic_example)
    + [User code](#user_code)
    + [Output](#output)

## Basic example <a name = "basic_example"></a>

### User code <a name = "user_code"></a>

```c
#define SUPOZI_IMPLEMENTATION
#include "supozi.h"
TEST(void, test_foo) {
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
```

### Output <a name = "output"></a>

```console
Running all test suites...
[  Suite  ] suite default, 3 tests
 => test default::test_addition ... ok
 => test default::test_subtraction ... ok
 => test default::test_foo ... FAILED
[  Suite  ] {default}: All tests completed. Failures: {1}

failures:

---- default::test_foo stdout ----
FOO
---- default::test_foo stderr ----

failures:
    default::test_foo: exit code {1}

test result: FAILED. 2 passed; 1 failed; elapsed: 0.04s
[ FAILED  ] Failures: {1}
[ DONE    ]
All tests completed. Failures: {1}
```
