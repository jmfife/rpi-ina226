/** Accum Avg - Accumulate and time-average values of interval data samples
 *  Copyright (C) 2022 John Michael Fife
 *
 *  Assumes timestamps are end - of - interval (EoI).
 *  Average accumulated value is duration-weighted based on all of the
 *  measurements made during the accumulation interval.
 */

#ifndef ACCUM_AVG_H_
#define ACCUM_AVG_H_ 

struct AccumAvg;
typedef struct AccumAvg AccumAvg;
AccumAvg* accum_avg_create();
void accum_avg_destroy(AccumAvg* self);
void accum_avg_accum(AccumAvg* self, double t, float val);
float accum_avg_yield(AccumAvg* self);
void accum_avg_reset(AccumAvg* self);
void accum_avg_reset2(AccumAvg* self, double t);

#endif
