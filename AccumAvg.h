#pragma once

/**
 * Accumulator for calculating averages over time intervals
 */
class AccumAvg {
  private:
	double t_initial;
	double t_last;
	float accum_total;
	int initialized = false;
  public:
	AccumAvg();
	void accum(double t, float val);
	float avg();
	void reset();
	void reset(double t);
};
