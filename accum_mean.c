/** Accum Mean - Accumulate and mean values of interval data samples
 *  Copyright (C) 2022 John Michael Fife
 *
 *  Assumes timestamps are end - of - interval (EoI).
 */

#include <stdio.h>
#include <stdlib.h>
#include "accum_mean.h"

struct AccumMean {
	uint32_t n;			// number of samples accumulated
	float accum_total;	// sum total of sample values
};

/* Initialize an accumulator interval.  */
void accum_mean_init(AccumMean* self_p) {
    self_p->n = 0;
    self_p->accum_total = 0;
}

/* Create an accumulator instance.  */
AccumMean* accum_mean_create() {
   AccumMean* result = (AccumMean*) malloc(sizeof(AccumMean));
   accum_mean_init(result);
   return result;
}

/* Destroy an accumulator instance.  */
void accum_mean_destroy(AccumMean* self_p) {
    free(self_p);
}

/* Accumulate a value into an accumulator instance.  */
void accum_mean_accum(AccumMean* self_p, double t, float val) {
	// Accumulate data point.  If not initialized, use this timestamp as initial time and disregard value.
	self_p->accum_total = self_p->accum_total + val;
	self_p->n += 1;
}

/* Find mean of accumulated values.  */
float accum_mean_yield(AccumMean* self_p) {
	// Return the average.  If not initialized, return the present accum_total.
	if (self_p->n != 0) {
		return self_p->accum_total / self_p->n;
	}else {
		return 0;
	}
}

/* Reset an accumulator instance */
void accum_mean_reset(AccumMean* self_p) {
	// Reset (re-initialize) the accumulator.
	accum_mean_init(self_p);
}

/* Reset an accumulator instance to a specfic timestamp. */
void accum_mean_reset2(AccumMean* self_p, double t) {
	// Reset (re-initialize) to time t.
	// For the mean, time doesn't matter
	accum_mean_init(self_p);
}

