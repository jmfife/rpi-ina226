#include "AccumAvg.h"
#include <iostream>
using namespace std;

int main()
{
  float a;
  std::cout << "Hello World!" << endl;
  AccumAvg f;
  f.reset(10.0);
  f.accum(20.0, 55.0);
  f.accum(35.0, 85.0);
  printf("Avg: %g should be equal to %g\n", f.avg(), (55.0*10.0 + 85.0*15.0)/25.0);
}
