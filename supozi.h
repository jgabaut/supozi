// jgabaut @ github.com/jgabaut
// SPDX-License-Identifier: GPL-3.0-only
/*
    Copyright (C) 2025 jgabaut

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef SUPOZI_H
#define SUPOZI_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#ifndef SPZ_NOPIPE
#include <unistd.h>
#include <sys/wait.h>
#endif // SPZ_NOPIPE

#define SPZ_MAJOR 0 /**< Represents current major release.*/
#define SPZ_MINOR 1 /**< Represents current minor release.*/
#define SPZ_PATCH 1 /**< Represents current patch release.*/

static const int SPZ_API_VERSION_INT =
    (SPZ_MAJOR * 1000000 + SPZ_MINOR * 10000 + SPZ_PATCH * 100);

typedef void (*test_void_fn)(void);
typedef int (*test_int_fn)(void);
typedef bool (*test_bool_fn)(void);

// Macro to declare a test function
#define TEST(retType, name) \
    static retType name(void); \
    static retType name(void)

#define ERROR_UNSUPPORTED_TYPE (*(int*)0)  // Passed an unexpected test type to REGISTER_TEST()

#define REGISTER_TEST(name) _Generic( (name), \
        test_void_fn: register_void_test, \
        test_int_fn: register_int_test, \
        test_bool_fn: register_bool_test, \
        default: ERROR_UNSUPPORTED_TYPE \
        )(#name, &name)

#define REGISTER_SUITE(name) \
    register_test_suite(name)

#ifndef SPZ_NOPIPE
#ifndef REGISTER_ALL_TESTS_PIPED
#define REGISTER_ALL_TESTS_PIPED 1
#endif // REGISTER_ALL_TESTS_PIPED
#ifndef SPZ_STDOUT_SUFFIX
#define SPZ_STDOUT_SUFFIX ".stdout"
#endif // SPZ_STDOUT_SUFFIX
#ifndef SPZ_STDERR_SUFFIX
#define SPZ_STDERR_SUFFIX ".stderr"
#endif // SPZ_STDERR_SUFFIX
#else
#ifndef REGISTER_ALL_TESTS_PIPED
#define REGISTER_ALL_TESTS_PIPED 0
#endif // REGISTER_ALL_TESTS_PIPED
#endif // SPZ_NOPIPE

#ifndef SPZ_NOPIPE
// Macro to register all tests automatically
#define REGISTER_ALL_TESTS() \
    static void register_all_tests(void) { \
        REGISTER_SUITE("default"); \
        TEST_LIST \
    } \
    static void spz_usage(const char* progname) { \
        if (!progname) return; \
        printf("Usage: %s [subcommand | SUITE | SUITE::TEST]\n", progname); \
        printf("\nArguments:\n\n"); \
        printf("  [subcommand]    record, help\n"); \
        printf("  SUITE           name of suite to run\n"); \
        printf("  SUITE::TEST     name of test to run from given suite\n"); \
        printf("\nSubcommands:\n\n"); \
        printf("  record          record all successful tests\n"); \
        printf("  help            show this message\n"); \
    } \
    /* Automatically generate the main function */ \
    int main(int argc, char** argv) { \
        printf("%s: using supozi v%i.%i.%i\n", argv[0], SPZ_MAJOR, SPZ_MINOR, SPZ_PATCH); \
        register_all_tests(); \
        if (argc > 1) { \
            if (!strcmp(argv[1], "help")) { \
                spz_usage(argv[0]); \
                return 0; \
            } else if (!strcmp(argv[1], "record")) { \
                return run_tests_record(REGISTER_ALL_TESTS_PIPED, 1, SPZ_STDOUT_SUFFIX, SPZ_STDERR_SUFFIX); \
            } else { \
                char namebuf[FILENAME_MAX] = {0}; \
                for (int i=0; i < SPZ_TEST_REGISTRY__.suites_count+1; i++) { \
                    TestSuite suite = SPZ_TEST_REGISTRY__.suites[i]; \
                    if (!strcmp(argv[1], suite.name)) { \
                        printf("%s: running suite %s:\n", argv[0], suite.name); \
                        int res = run_suite(suite, REGISTER_ALL_TESTS_PIPED); \
                        return res; \
                    } \
                    for (int j=0; j < suite.test_count+1; j++) { \
                        Test t = suite.tests[j]; \
                        sprintf(namebuf, "%s::%s", suite.name, t.name); \
                        if (!strcmp(argv[1], namebuf)) { \
                            printf("%s: running test %s::%s: ", argv[0], suite.name, t.name); \
                            fflush(stdout); \
                            int res = -1; \
                            TestResult tr = {0}; \
                            if (REGISTER_ALL_TESTS_PIPED == 1) { \
                                tr = run_test_piped(t); \
                                res = tr.exit_code; \
                            } else { \
                                res = run_test(t); \
                            } \
                            printf("%s\n", (res == 0 ? "\033[0;32mSUCCESS\033[0m" : "\033[0;31mFAILURE\033[0m")); \
                            if (REGISTER_ALL_TESTS_PIPED == 1) { \
                                printf("---- %s::%s stdout ----\n", suite.name, t.name); \
                                spz_print_stream_to_file(tr.stdout_pipe, stdout); \
                                printf("---- %s::%s stderr ----\n", suite.name, t.name); \
                                spz_print_stream_to_file(tr.stderr_pipe, stdout); \
                                close(tr.stdout_pipe); \
                                close(tr.stderr_pipe); \
                            } \
                            return res; \
                        } \
                    } \
                } \
                printf("%s: unknown argument: %s\n", argv[0], argv[1]); \
                spz_usage(argv[0]); \
                return 1; \
            } \
        } else { \
            return run_tests(REGISTER_ALL_TESTS_PIPED); \
        } \
    }
#else
#define REGISTER_ALL_TESTS() \
    static void register_all_tests(void) { \
        REGISTER_SUITE("default"); \
        TEST_LIST \
    } \
    static void spz_usage(const char* progname) { \
        if (!progname) return; \
        printf("Usage: %s [subcommand | SUITE | SUITE::TEST]\n", progname); \
        printf("\nArguments:\n\n"); \
        printf("  [subcommand]    record, help\n"); \
        printf("  SUITE           name of suite to run\n"); \
        printf("  SUITE::TEST     name of test to run from given suite\n"); \
        printf("\nSubcommands:\n\n"); \
        printf("  help            show this message\n"); \
    } \
    /* Automatically generate the main function */ \
    int main(int argc, char** argv) { \
        printf("%s: using supozi v%i.%i.%i\n", argv[0], SPZ_MAJOR, SPZ_MINOR, SPZ_PATCH); \
        register_all_tests(); \
        if (argc > 1) { \
            if (!strcmp(argv[1], "help")) { \
                spz_usage(argv[0]); \
                return 0; \
            } else { \
                char namebuf[FILENAME_MAX] = {0}; \
                for (int i=0; i < SPZ_TEST_REGISTRY__.suites_count+1; i++) { \
                    TestSuite suite = SPZ_TEST_REGISTRY__.suites[i]; \
                    if (!strcmp(argv[1], suite.name)) { \
                        printf("%s: running suite %s:\n", argv[0], suite.name); \
                        int res = run_suite(suite, REGISTER_ALL_TESTS_PIPED); \
                        return res; \
                    } \
                    for (int j=0; j < suite.test_count+1; j++) { \
                        Test t = suite.tests[j]; \
                        sprintf(namebuf, "%s::%s", suite.name, t.name); \
                        if (!strcmp(argv[1], namebuf)) { \
                            printf("%s: running test %s::%s: ", argv[0], suite.name, t.name); \
                            fflush(stdout); \
                            int res = run_test(t); \
                            printf("%s\n", (res == 0 ? "\033[0;32mSUCCESS\033[0m" : "\033[0;31mFAILURE\033[0m")); \
                            return res; \
                        } \
                    } \
                } \
                printf("%s: unknown argument: %s\n", argv[0], argv[1]); \
                spz_usage(argv[0]); \
                return 1; \
            } \
        } else { \
            return run_tests(REGISTER_ALL_TESTS_PIPED); \
        } \
    }
#endif // SPZ_NOPIPE

typedef union test_fn {
    test_void_fn void_fn;
    test_int_fn int_fn;
    test_bool_fn bool_fn;
} test_fn;

typedef enum Test_Type {
    TEST_VOID,
    TEST_INT,
    TEST_BOOL,
} Test_Type;

typedef struct Test {
    Test_Type type;
    test_fn func;
    const char* name;
} Test;

#define MAX_TESTS 100
typedef struct TestSuite {
    Test tests[MAX_TESTS];
    int test_count;
    const char* name;
} TestSuite;

#define MAX_SUITES 100
typedef struct TestRegistry {
    TestSuite suites[MAX_SUITES];
    int suites_count;
} TestRegistry;

extern TestRegistry SPZ_TEST_REGISTRY__;

#ifndef _WIN32
#define SPZ_PATH_SEPARATOR "/"
#else
#define SPZ_PATH_SEPARATOR "\\"
#endif // _WIN32

// Functions to register tests explicitly to global SPZ_TEST_REGISTRY__
void register_void_test(const char* name, test_void_fn func);
void register_int_test(const char* name, test_int_fn func);
void register_bool_test(const char* name, test_bool_fn func);
void register_test_suite(const char* name);
// Functions to register tests explicitly to a specific registry
void register_bool_test_toreg(TestRegistry *tr, const char* name, test_bool_fn func);
void register_int_test_toreg(TestRegistry *tr, const char* name, test_int_fn func);
void register_void_test_toreg(TestRegistry *tr, const char* name, test_void_fn func);
void register_test_suite_toreg(TestRegistry *tr, const char* name);
// Function to run a single test (see also run_test_piped())
int run_test(Test t);
// Functions to run all tests in a suite
int run_suite(TestSuite suite, int piped);
int run_suite_record(TestSuite suite, int piped, int record, const char* stdout_record_suffix, const char* stderr_record_suffix);
// Functions to run all tests in global SPZ_TEST_REGISTRY__
int run_tests(int piped);
int run_tests_record(int piped, int record, const char* stdout_record_suffix, const char* stderr_record_suffix);
// Functions to run all tests in a specific registry
int run_testregistry(TestRegistry tr, int piped);
int run_testregistry_record(TestRegistry tr, int piped, int record, const char* stdout_record_suffix, const char* stderr_record_suffix);

#ifndef SPZ_NOPIPE

typedef struct TestResult {
    int exit_code;
    int stdout_pipe;
    int stderr_pipe;
    int signum;
} TestResult;

TestResult run_test_piped(Test t); // Caller must close TestResult.stdout_pipe and TestResult.stderr_pipe

typedef TestResult CmdResult;
CmdResult run_cmd_piped(const char* cmd); // Caller must close CmdResult.stdout_pipe and CmdResult.stderr_pipe
#endif // SPZ_NOPIPE

#ifndef SPZ_NOTIMER
#ifndef DUMBTIMER_H_
#define DUMBTIMER_H_
#include <time.h>
#ifdef _WIN32
#include <profileapi.h>		//Used for QueryPerformanceFrequency(), QueryPerformanceCounter()
#endif

#define DUMBTIMER_MAJOR 0 /**< Represents current major release.*/
#define DUMBTIMER_MINOR 1 /**< Represents current minor release.*/
#define DUMBTIMER_PATCH 0 /**< Represents current patch release.*/

static const int DUMBTIMER_API_VERSION_INT =
    (DUMBTIMER_MAJOR * 1000000 + DUMBTIMER_MINOR * 10000 + DUMBTIMER_PATCH * 100);

struct DumbTimer {
#ifndef _WIN32
    struct timespec start_time, end_time;
#else
    LARGE_INTEGER start_time, end_time, frequency;
#endif
};

typedef struct DumbTimer DumbTimer;

#ifndef _WIN32
#define DumbTimer_Fmt "DumbTimer { start_time: %.9f, end_time: %.9f }"
#define DumbTimer_Arg(dt) \
    ((dt)->start_time.tv_sec + (dt)->start_time.tv_nsec / 1e9), \
    ((dt)->end_time.tv_sec + (dt)->end_time.tv_nsec / 1e9)
#else
#define DumbTimer_Fmt "DumbTimer { start_time: %.7f, end_time: %.7f, frequency: %.7f }"
#define DumbTimer_Arg(dt) \
    ((double)(dt)->start_time.QuadPart / (dt)->frequency.QuadPart), \
    ((double)(dt)->end_time.QuadPart / (dt)->frequency.QuadPart), \
    ((double)(dt)->frequency.QuadPart)
#endif

#define DUMBTIMER_TIMED(func, res, elapsed, ...) do { \
    DumbTimer dt__ = dt_new(); \
    (*res) = (func)(__VA_ARGS__); \
    (*elapsed) = dt_stop(&dt__); \
} while (0)

DumbTimer dt_new(void);
double dt_elapsed(DumbTimer* dt);
double dt_stop(DumbTimer* dt);
#endif // DUMBTIMER_H_
#endif // SPZ_NOTIMER

#endif // SUPOZI_H

#ifdef SPZ_IMPLEMENTATION

TestRegistry SPZ_TEST_REGISTRY__ = { .suites_count = -1, };

#define register_test(registry, test_type, name, func) do { \
    TestSuite* curr_suite = &(registry->suites[registry->suites_count]); \
    /* printf("%s(): Registering test {%s} to suite {%s}\n", __func__, name, curr_suite->name); */\
    if (curr_suite->test_count < MAX_TESTS) { \
        curr_suite->tests[curr_suite->test_count].type = _Generic(((test_type)0), \
                void*: TEST_VOID, \
                int: TEST_INT, \
                bool: TEST_BOOL, \
                default: TEST_VOID \
                ); \
        curr_suite->tests[curr_suite->test_count].func.test_type##_fn = func; \
        curr_suite->tests[curr_suite->test_count].name = name; \
        curr_suite->test_count++; \
    } else { \
        fprintf(stderr, "%s(): can't accept {%s}, suite {%s} is full\n", __func__, name, curr_suite->name); \
    } \
} while (0)

void register_bool_test_toreg(TestRegistry *tr, const char* name, test_bool_fn func) {
    register_test(tr, bool, name, func);
}

void register_void_test_toreg(TestRegistry *tr, const char* name, test_void_fn func) {
    register_test(tr, void, name, func);
}

void register_int_test_toreg(TestRegistry *tr, const char* name, test_int_fn func) {
    register_test(tr, int, name, func);
}

void register_bool_test(const char* name, test_bool_fn func) {
    register_bool_test_toreg(&SPZ_TEST_REGISTRY__, name, func);
}

void register_void_test(const char* name, test_void_fn func) {
    register_void_test_toreg(&SPZ_TEST_REGISTRY__, name, func);
}

void register_int_test(const char* name, test_int_fn func) {
    register_int_test_toreg(&SPZ_TEST_REGISTRY__, name, func);
}

void register_test_suite_toreg(TestRegistry *tr, const char* name) {
    if (tr->suites_count < MAX_SUITES) {
        tr->suites_count++;
        tr->suites[tr->suites_count].name = name;
    } else {
        fprintf(stderr, "%s(): can't accept suite {%s}, registry is full\n", __func__, name);
    }
}

void register_test_suite(const char* name) {
    register_test_suite_toreg(&SPZ_TEST_REGISTRY__, name);
}

int run_test(Test t) {
    int res = 0;
    switch (t.type) {
        case TEST_VOID: {
            t.func.void_fn();
        }
        break;
        case TEST_INT: {
            int ires = t.func.int_fn();
            res = ires;
        }
        break;
        case TEST_BOOL: {
            bool bres = t.func.bool_fn();
            res = !bres;
        }
        break;
        default: {

        }
        break;
    }
    return res;
}

#ifndef SPZ_NOPIPE
static inline int spz_call_cmd(const char* x) {
    return execlp(x, x, (char*) NULL);
}

static inline int spz_call_test(Test x) {
    return run_test(x);
}

#define run_piped__(retType, x) do { \
    int stdout_pipe[2], stderr_pipe[2]; \
    if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) { \
        perror("pipe"); \
        exit(EXIT_FAILURE); \
    } \
    pid_t pid = fork(); \
    if (pid == -1) { \
        perror("fork"); \
        exit(EXIT_FAILURE); \
    } \
    if (pid == 0) { \
        /* Child process*/ \
        /* Close read end of stdout pipe */ \
        close(stdout_pipe[0]);  \
        /* Close read end of stderr pipe */ \
        close(stderr_pipe[0]);  \
        /* Redirect stdout to pipe */ \
        dup2(stdout_pipe[1], STDOUT_FILENO); \
        /* Redirect stderr to pipe */ \
        dup2(stderr_pipe[1], STDERR_FILENO); \
        /* Close write end after duplicating */ \
        close(stdout_pipe[1]); \
        close(stderr_pipe[1]); \
        int res = _Generic((x), \
                const char*: spz_call_cmd, \
                Test: spz_call_test, \
                default: ERROR_UNSUPPORTED_TYPE \
                )(x); \
        exit(res); \
    } else { \
        /* Parent process */ \
        /* Close write end of stdout pipe */ \
        close(stdout_pipe[1]); \
        /* Close write end of stderr pipe */ \
        close(stderr_pipe[1]); \
        /* Wait for child process to finish */ \
        int status; \
        if (waitpid(pid, &status, 0) == -1) { \
            fprintf(stderr, "%s(): waitpid() failed\n", __func__); \
            close(stdout_pipe[0]); \
            close(stderr_pipe[0]); \
            return (retType) { \
                .exit_code = -1, \
                .stdout_pipe = -1, \
                .stderr_pipe = -1, \
                .signum = -1, \
            }; \
        }; \
        int es = -1; \
        if ( WIFEXITED(status) ) { \
            es = WEXITSTATUS(status); \
        } \
        int signal = -1; \
        if (WIFSIGNALED(status)) { \
            signal = WTERMSIG(status); \
            printf("%s(): process was terminated by signal %i\n", __func__, signal); \
        } \
        return (retType) { \
            .exit_code = es, \
            /* Must be closed by caller */ \
            .stdout_pipe = stdout_pipe[0], \
            /* Must be closed by caller */ \
            .stderr_pipe = stderr_pipe[0], \
            .signum = signal, \
        }; \
    } \
} while(0)

TestResult run_test_piped(Test t) {
    run_piped__(TestResult, t);
}

CmdResult run_cmd_piped(const char* cmd) {
    run_piped__(CmdResult, cmd);
}

#define run_piped(x) _Generic((x), \
        const char*: run_cmd_piped, \
        Test: run_test_piped, \
        default: ERROR_UNSUPPORTED_TYPE \
        )(x)

static inline void spz_print_stream_to_file(int source, FILE* dest)
{
    if (!dest) return;
    char buffer[256];
    ssize_t count;
    while ((count = read(source, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[count] = '\0';
        fprintf(dest, "%s", buffer);
    }
}
#endif // SPZ_NOPIPE

int run_suite(TestSuite suite, int piped) {
    return run_suite_record(suite, piped, 0, NULL, NULL);
}

int run_suite_record(TestSuite suite, int piped, int record, const char* stdout_record_suffix, const char* stderr_record_suffix) {
    int failures = 0;
    int successes = 0;
#ifndef SPZ_NOPIPE
    int exit_codes[MAX_TESTS] = {0};
    int error_streams[MAX_TESTS][2] = {0};
    const char* failed[MAX_TESTS] = {0};
#endif // SPZ_NOPIPE

#ifndef SPZ_NOTIMER
    DumbTimer timer = dt_new();
#endif // SPZ_NOTIMER

    for (int i = 0; i < suite.test_count; i++) {
        printf(" => test %s::%s ... ", suite.name, suite.tests[i].name);
        fflush(stdout);
#ifndef SPZ_NOPIPE
        if (piped > 0) {
            TestResult res = run_test_piped(suite.tests[i]);
            if (res.exit_code != 0) {
                printf("\033[0;31mFAILED\033[0m\n");
                exit_codes[failures] = res.exit_code;
                error_streams[failures][0] = res.stdout_pipe;
                error_streams[failures][1] = res.stderr_pipe;
                failed[failures] = suite.tests[i].name;
                failures++;
            } else {
                printf("\033[0;32mok\033[0m\n");
                successes++;
                if (record > 0) {
                    char pathbuf[FILENAME_MAX] = {0};
                    const char* stdout_pb_suffix = NULL;
                    if (!stdout_record_suffix) {
                        stdout_pb_suffix = ".stdout";
                    } else {
                        stdout_pb_suffix = stdout_record_suffix;
                    }
                    sprintf(pathbuf, ".%s%s%s", SPZ_PATH_SEPARATOR, suite.tests[i].name, stdout_pb_suffix);
                    FILE* stdout_record_file = fopen(pathbuf, "w");
                    spz_print_stream_to_file(res.stdout_pipe, stdout_record_file);
                    fclose(stdout_record_file);

                    const char* stderr_pb_suffix = NULL;
                    if (!stderr_record_suffix) {
                        stderr_pb_suffix = ".stderr";
                    } else {
                        stderr_pb_suffix = stderr_record_suffix;
                    }
                    sprintf(pathbuf, ".%s%s%s", SPZ_PATH_SEPARATOR, suite.tests[i].name, stderr_pb_suffix);
                    FILE* stderr_record_file = fopen(pathbuf, "w");
                    spz_print_stream_to_file(res.stderr_pipe, stderr_record_file);
                    fclose(stderr_record_file);
                }
                close(res.stdout_pipe);
                close(res.stderr_pipe);
            }
        } else {
            int res = run_test(suite.tests[i]);
            if (res != 0) {
                printf("\033[0;31mFAILED\033[0m, res: {%d}\n", res);
                failures++;
            } else {
                printf("\033[0;32mok\033[0m\n");
                successes++;
            }
        }
#else
        int res = run_test(suite.tests[i]);
        if (res != 0) {
            printf("\033[0;31mFAILED\033[0m, res: {%d}\n", res);
            failures++;
        } else {
            printf("\033[0;32mok\033[0m\n");
            successes++;
        }
#endif // SPZ_NOPIPE
    }

#ifndef SPZ_NOTIMER
    double elapsed = dt_stop(&timer);
#endif // SPZ_NOTIMER

    printf("[  Suite  ] {%s}: All tests completed. Failures: {%d}\n", suite.name, failures);

#ifndef SPZ_NOPIPE
    if (piped > 0) {
        printf("\nfailures:\n\n");
        for (int i=0; i < failures; i++) {
            printf("---- %s::%s stdout ----\n", suite.name, failed[i]);
            spz_print_stream_to_file(error_streams[i][0], stdout);

            printf("---- %s::%s stderr ----\n", suite.name, failed[i]);
            spz_print_stream_to_file(error_streams[i][1], stdout);

            close(error_streams[i][0]);
            close(error_streams[i][1]);
        }
        printf("\nfailures:\n");
        for (int i=0; i < failures; i++) {
            printf("    %s::%s: exit code {%i}\n", suite.name, failed[i], exit_codes[i]);
        }
    }
#endif // SPZ_NOPIPE
#ifndef SPZ_NOTIMER
    printf("\ntest result: %s. %i passed; %i failed; elapsed: %.2fs\n", (failures == 0 ? "\033[0;32PASSED\033[0m" : "\033[0;31mFAILED\033[0m"), successes, failures, elapsed);
#else
    printf("\ntest result: %s. %i passed; %i failed;\n", (failures == 0 ? "\033[0;32PASSED\033[0m" : "\033[0;31mFAILED\033[0m"), successes, failures);
#endif // SPZ_NOTIMER
    return failures;
}

int run_tests(int piped) {
    return run_tests_record(piped, 0, NULL, NULL);
}

int run_tests_record(int piped, int record, const char* stdout_record_suffix, const char* stderr_record_suffix) {
    return run_testregistry_record(SPZ_TEST_REGISTRY__, piped, record, stdout_record_suffix, stderr_record_suffix);
}

int run_testregistry(TestRegistry tr, int piped) {
    return run_testregistry_record(tr, piped, 0, NULL, NULL);
}

int run_testregistry_record(TestRegistry tr, int piped, int record, const char* stdout_record_suffix, const char* stderr_record_suffix) {
    int failures = 0;
    printf("Running all test suites...\n");
    for (int i = 0; i < tr.suites_count+1; i++) {
        TestSuite suite = tr.suites[i];
        printf("[  Suite  ] suite %s, %d tests\n", suite.name, suite.test_count);
        int res = run_suite_record(suite, piped, record, stdout_record_suffix, stderr_record_suffix);
        if (res > 0) {
            printf("[ FAILED  ] Failures: {%d}\n", res);
        } else {
            printf("[ SUCCESS ]\n");
        }
        failures += res;
        printf("[ DONE    ]\n");
    }
    printf("All tests completed. Failures: {%d}\n", failures);
    return failures;
}

#ifndef SPZ_NOTIMER
DumbTimer dt_new(void)
{
    DumbTimer self = {0};
#ifndef _WIN32
    clock_gettime(CLOCK_MONOTONIC, &(self.start_time));
#else
    QueryPerformanceFrequency(&(self.frequency));
    QueryPerformanceCounter(&(self.start_time));
#endif // _WIN32
    return self;
}

double dt_elapsed(DumbTimer* dt)
{
#ifndef _WIN32
    struct timespec end = {0};
    clock_gettime(CLOCK_MONOTONIC, &end);	// %.9f
    double elapsed_time =
        (end.tv_sec - dt->start_time.tv_sec) + (end.tv_nsec -
            dt->start_time.tv_nsec) / 1e9;
#else
    LARGE_INTEGER end = {0};
    QueryPerformanceCounter(&end);	// %.7f
    double elapsed_time =
        (double)(end.QuadPart -
                 dt->start_time.QuadPart) / dt->frequency.QuadPart;
#endif
    return elapsed_time;
}

double dt_stop(DumbTimer* dt)
{
#ifndef _WIN32
    clock_gettime(CLOCK_MONOTONIC, &(dt->end_time));
    double elapsed_time =
        (dt->end_time.tv_sec - dt->start_time.tv_sec) + (dt->end_time.tv_nsec -
            dt->start_time.tv_nsec) / 1e9;
#else
    QueryPerformanceCounter(&(dt->end_time));
    double elapsed_time =
        (double)(dt->end_time.QuadPart -
                 dt->start_time.QuadPart) / dt->frequency.QuadPart;
#endif
    return elapsed_time;
}

#endif // SPZ_NOTIMER

// Cleanup
#undef register_test
#undef run_piped__
#endif // SPZ_IMPLEMENTATION
