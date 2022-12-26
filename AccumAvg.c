// accumulate and average values for interval data processing
#include <stdio.h>
#include <stdlib.h>
#include "AccumAvg.h"

struct AccumAvg {
    double t_initial;
    double t_last;
    float accum_total;
    int initialized;
};

// Constructor (without allocation)
void AccumAvg_init(AccumAvg* self) {
    self->t_initial = 0;
    self->t_last = 0;
    self->accum_total = 0;
    self->initialized = 0;
}

// Allocation + initialization (equivalent to "new Point(x, y)")
AccumAvg* AccumAvg_create() {
   AccumAvg* result = (AccumAvg*) malloc(sizeof(AccumAvg));
   AccumAvg_init(result);
   return result;
}

// Destructor + deallocation (equivalent to "delete point")
void AccumAvg_destroy(AccumAvg* self) {
    free(self);
}

void AccumAvg_accum(AccumAvg* self, double t, float val) {
    // Accumulate data point.  If not initialized, use this timestamp as initial time and disregard value.
	float avg;
	if (self->initialized) {
		self->accum_total = self->accum_total + val*(t-self->t_last);
	} else {
		self->t_initial = t;
	}
	self->t_last = t;
	self->initialized = 1;
}

float AccumAvg_avg(AccumAvg* self) {
	// Return the average.  If not initialized, return the present accum_total.
	float dt = self->t_last - self->t_initial;
	if (dt != 0.0) {
		return self->accum_total / (self->t_last - self->t_initial);
	} else {
		return self->accum_total;
	}
}

void AccumAvg_reset(AccumAvg* self) {
	// Reset (re-initialize) to last value accumulated.
	self->accum_total = 0;
	self->t_initial = self->t_last;
	self->initialized = 1;
}

void AccumAvg_reset2(AccumAvg* self, double t) {
	// Reset (re-initialize) to time t.
	self->accum_total = 0;
	self->t_initial = t;
	self->t_last = t;
	self->initialized = 1;
}

