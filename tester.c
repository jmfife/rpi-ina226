#define _GNU_SOURCE     /* get strsignal def */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>

#include "tester.h"

struct tester_global_data TesterData;

const char *
testerStrsignal(int sig)
{
    return strsignal(sig);
}

static void
tester_sighandler(int signal)
{
    if(TesterData.escape_hatch_active) {
        TesterData.escape_hatch_active = 0;
        longjmp(TesterData.escape_hatch, signal);
    }
}

void
testerInit(void)
{
    TesterData.escape_hatch_active = 0;
    TesterData.tests = 0;
    TesterData.errors = 0;
    TesterData.signals = 0;
    TesterData.user_fails = 0;

    signal(SIGSEGV, tester_sighandler);
    signal(SIGILL, tester_sighandler);
    signal(SIGFPE, tester_sighandler);
    signal(SIGALRM, tester_sighandler);
    signal(SIGBUS, tester_sighandler);
    signal(SIGABRT, tester_sighandler);
}

void
testerReport(FILE *f, const char *preamble)
{
    if(TesterData.errors != 0 || TesterData.signals != 0) {
        fprintf(f, "%s: errors %d/%d, signals %d, FAILs %d\n", 
                preamble,
                TesterData.errors, 
                TesterData.tests,
                TesterData.signals,
                TesterData.user_fails);
    }
}

int
testerResult(void)
{
    return TesterData.errors;
}

void
tester_set_time_limit(int t)
{
    TesterData.time_limit = t;
}
