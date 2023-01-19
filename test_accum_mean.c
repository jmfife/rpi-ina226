#include "accum_mean.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include "tester.h"

int main(int argc, char *argv[])
{
  AccumMean* f;
  float expected;
  TRY{ f = accum_mean_create(); } ENDTRY;
  TEST_ASSERT(accum_mean_yield(f) == 0.0);
  TRY{ accum_mean_reset2(f, 10.0); } ENDTRY;
  TRY{ accum_mean_accum(f, 20.0, 55.0); } ENDTRY;
  TRY{ accum_mean_accum(f, 35.0, 85.0); } ENDTRY;
  expected = (55.0 + 85.0) / 2.0;
  // printf("Mean: %g should be equal to %g\n", accum_mean_yield(f), expected);
  TEST_ASSERT(fabs(accum_mean_yield(f) - expected) < fabs(expected*0.001));
  TRY{ accum_mean_destroy(f); } ENDTRY;
  testerReport(stdout, argv[0]);
  return testerResult();
}


        