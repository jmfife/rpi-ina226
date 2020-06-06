// accumulate and average values for interval data processing
#include <stdio.h>
#include "AccumAvg.h"

AccumAvg::AccumAvg() {
	t_initial = 0;
	t_last = 0; 
	accum_total = 0;
	initialized = false;
};
void AccumAvg::accum(double t, float val) {
	// Accumulate data point.  If not initialized, use this timestamp as initial time and disregard value.
	float avg;
	if (initialized) {
		accum_total = accum_total + val*(t-t_last);
	} else {
		t_initial = t;
	}
	t_last = t;
	initialized = true;
}
float AccumAvg::avg() {
	// Return the average.  If not initialized, return the present accum_total.
	float dt = t_last - t_initial;
	if (dt != 0.0) {
		return accum_total / (t_last - t_initial);
	} else {
		return accum_total;
	}
}
void AccumAvg::reset() {
	// Reset (re-initialize) to last value accumulated.
	accum_total = 0;
	t_initial = t_last;
	initialized = true;
}
void AccumAvg::reset(double t) {
	// Reset (re-initialize) to time t.
	accum_total = 0;
	t_initial = t;
	t_last = t;
	initialized = true;
}

