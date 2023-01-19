/** Accum Mean - Accumulate and average values of interval data samples
 *  Copyright (C) 2022 John Michael Fife
 *
 *  Assumes timestamps are end - of - interval (EoI).
 */

#ifndef ACCUMMEAN_H_
#define ACCUMMEAN_H_ 

struct AccumMean;
typedef struct AccumMean AccumMean;
AccumMean* accum_mean_create();
void accum_mean_destroy(AccumMean* self);
void accum_mean_accum(AccumMean* self, double t, float val);
float accum_mean_yield(AccumMean* self);
void accum_mean_reset(AccumMean* self);
void accum_mean_reset2(AccumMean* self, double t);

#endif
