/** AccumAvg - Accumulate and time-average values of interval data samples
 *  Copyright (C) 2022 John Michael Fife
 *
 *  Assumes timestamps are end - of - interval (EoI).
 */

#ifndef ACCUMAVG_H_
#define ACCUMAVG_H_ 

struct AccumAvg;
typedef struct AccumAvg AccumAvg;
AccumAvg* AccumAvg_create();
void AccumAvg_destroy(AccumAvg* self);
void AccumAvg_accum(AccumAvg* self, double t, float val);
float AccumAvg_avg(AccumAvg* self);
void AccumAvg_reset(AccumAvg* self);
void AccumAvg_reset2(AccumAvg* self, double t);
	
#endif
