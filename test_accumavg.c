#include "AccumAvg.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include "tester.h"

int main(int argc, char *argv[])
{
  AccumAvg* f;
  float expected;
  TRY{ f = AccumAvg_create(); } ENDTRY;
  TEST_ASSERT(AccumAvg_avg(f) == 0.0);
  TRY{ AccumAvg_reset2(f, 10.0); } ENDTRY;
  TRY{ AccumAvg_accum(f, 20.0, 55.0); } ENDTRY;
  TRY{ AccumAvg_accum(f, 35.0, 85.0); } ENDTRY;
  expected = (55.0 * 10.0 + 85.0 * 15.0) / 25.0;
  // printf("Avg: %g should be equal to %g\n", AccumAvg_avg(f), expected);
  TEST_ASSERT(fabs(AccumAvg_avg(f) - expected) < fabs(expected*0.001));
  TRY{ AccumAvg_destroy(f); } ENDTRY;
  testerReport(stdout, argv[0]);
  return testerResult();
}


        