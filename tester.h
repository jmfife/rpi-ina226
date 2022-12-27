/*
 * Test macros.
 * 
 * Usage:
 *
 * #include <setjmp.h>
 * #include <stdio.h>
 * #include <signal.h>
 * #include <unistd.h>
 *
 * testerInit();                  -- Initialize internal data structures.
 * testerReport(FILE *, "name");  -- Print report.
 * testerResult();                -- Returns # of failed tests.
 *
 * TRY { code } ENDTRY;
 *
 * Wraps code to catch seg faults, illegal instructions, etc.  May not be
 * nested.
 * Prints a warning if a signal is caught.
 * To enforce a maximum time, set alarm before entering.
 *
 * TEST(expr, expected_value);
 *
 * Evaluates expr (which should yield an integer value) inside a TRY.
 * Prints a warning if evaluating expr causes a fault or returns a value
 * not equal to expected_value.
 *
 * TEST_ASSERT(expr)
 *
 * Equivalent to TEST(!(expr), 0)
 *
 * You can also cause your own failures with FAIL:
 *
 * TRY {
 *     x = 1;
 *     if(x == 2) FAIL("why is x 2?");
 * } ENDTRY;
 *
 * To limit the time taken by a test, call tester_set_time_limit with
 * a new limit in seconds, e.g.
 *
 * tester_set_time_limit(1);
 * TRY { while(1); } ENDTRY;
 *
 * There is an initial default limit of 10 seconds.
 * If you don't want any limit, set the limit to 0.
 *
 */

/* global data used by macros */
/* nothing in here should be modified directly */
extern struct tester_global_data {
    jmp_buf escape_hatch;       /* jump here on surprise signals */
    int escape_hatch_active;    /* true if escape hatch is usable */
    int tests;                  /* number of tests performed */
    int errors;                 /* number of tests failed */
    int signals;                /* number of signals caught */
    int expr_value;             /* expression value */
    int setjmp_return;          /* return value from setjmp */
    int try_failed;             /* true if last try failed */
    int user_fails;             /* number of calls to FAIL */
    int time_limit;             /* time limit for TRY */
} TesterData;

/* set up system; call this before using macros */
void testerInit(void);

/* prints a summary report of all errors to f, prefixed with preamble */
/* If there were no errors, nothing is printed */
void testerReport(FILE *f, const char *preamble);

/* returns number of errors so far. */
int testerResult(void);

/* set a time limit t for TRY, TEST, TEST_ASSERT etc. */
/* After t seconds, an ALARM signal will interrupt the test. */
/* Set t = 0 to have no time limit. */
/* Default time limit is 10 seconds. */
void tester_set_time_limit(int t);        

const char *testerStrsignal(int);      /* internal hack; don't use this */

/* gruesome non-syntactic macros */
#define TRY \
    TesterData.try_failed = 0; \
    alarm(TesterData.time_limit); \
    if(((TesterData.setjmp_return = setjmp(TesterData.escape_hatch)) == 0) \
            && (TesterData.escape_hatch_active = 1) /* one = is correct*/)
#define ENDTRY else { \
        fprintf(stderr, "%s:%d: %s (signal %d)\n", \
            __FILE__, __LINE__, \
            testerStrsignal(TesterData.setjmp_return), \
            TesterData.setjmp_return); \
        TesterData.signals++; \
        TesterData.try_failed = 1; \
    } \
    alarm(0); \
    TesterData.escape_hatch_active = 0

/* another atrocity */
#define TEST(expr, expected_value) \
    TesterData.tests++; \
    TesterData.errors++; /* guilty until proven innocent */ \
    TRY { TesterData.expr_value = (expr); \
        if(TesterData.expr_value != expected_value) { \
            fprintf(stderr, "%s:%d: TEST FAILED: %s -> %d but expected %d\n", \
                    __FILE__, __LINE__, __STRING(expr), \
                    TesterData.expr_value, expected_value); \
        } else { \
            TesterData.errors--; \
        } \
    } \
    ENDTRY; \
    if(TesterData.try_failed) \
        fprintf(stderr, "%s:%d: TEST FAILED: %s caught signal\n", \
                __FILE__, __LINE__, __STRING(expr))

#define TEST_ASSERT(expr) TEST((expr) != 0, 1)
#define FAIL(msg) \
    (fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, (msg)), \
             TesterData.user_fails++, \
             TesterData.try_failed = 1)
