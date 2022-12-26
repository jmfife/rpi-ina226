#include "AccumAvg.h"
#include <stdio.h>

int main()
{
  float a;
  AccumAvg* f = AccumAvg_create();
  AccumAvg_reset2(f, 10.0);
  AccumAvg_accum(f, 20.0, 55.0);
  AccumAvg_accum(f, 35.0, 85.0);
  printf("Avg: %g should be equal to %g\n", AccumAvg_avg(f), (55.0*10.0 + 85.0*15.0)/25.0);
}
