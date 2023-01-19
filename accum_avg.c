/** AccumAvg - Accumulate and time-average values of interval data samples
 *  Copyright (C) 2022 John Michael Fife
 *
 *  Assumes timestamps are end - of - interval (EoI).
 *  Average accumulated value is duration-weighted based on all of the
 *  measurements made during the accumulation interval.
 */

#include <stdio.h>
#include <stdlib.h>
#include "accum_avg.h"

struct AccumAvg {
    double t_initial;
    double t_last;
    float accum_total;
    int initialized;
};

/* Initialize an accumulator instance.  */
void accum_avg_init(AccumAvg* self_p) {
    self_p->t_initial = 0;
    self_p->t_last = 0;
    self_p->accum_total = 0;
    self_p->initialized = 0;
}

/* Create an accumulator instance.  */
AccumAvg* accum_avg_create() {
   AccumAvg* result = (AccumAvg*) malloc(sizeof(AccumAvg));
   accum_avg_init(result);
   return result;
}

/* Destroy and accumulator instance.  */
void accum_avg_destroy(AccumAvg* self_p) {
    free(self_p);
}

/* Accumulate a value into an accumulator instance.  */
void accum_avg_accum(AccumAvg* self_p, double t, float val) {
	// Accumulate data point.  If not initialized, use this timestamp as initial time and disregard value.
	if (self_p->initialized) {
		self_p->accum_total = self_p->accum_total + val*(t-self_p->t_last);
	} else {
		self_p->t_initial = t;
	}
	self_p->t_last = t;
	self_p->initialized = 1;
}

/* Average accumulated values.  */
float accum_avg_yield(AccumAvg* self_p) {
	// Return the average.  If not initialized, return the present accum_total.
	float dt = self_p->t_last - self_p->t_initial;
	if (dt != 0.0) {
		return self_p->accum_total / dt;
	}else {
		return self_p->accum_total;
	}
}

/* Reset an accumulator instance */
void accum_avg_reset(AccumAvg* self_p) {
	// Reset (re-initialize) timer to last value accumulated.
	self_p->accum_total = 0;
	self_p->t_initial = self_p->t_last;
	self_p->initialized = 1;
}

/* Reset an accumulator instance to a specfic timestamp. */
void accum_avg_reset2(AccumAvg* self_p, double t) {
	// Reset (re-initialize) to time t.
	self_p->accum_total = 0;
	self_p->t_initial = t;
	self_p->t_last = t;
	self_p->initialized = 1;
}

