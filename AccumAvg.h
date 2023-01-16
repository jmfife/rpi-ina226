#pragma once

/**
 * Accumulator for calculating averages over time intervals
 */
// class AccumAvg {
//   private:
// 	double t_initial;
// 	double t_last;
// 	float accum_total;
// 	int initialized;
//   public:
// 	AccumAvg();
// 	void accum(double t, float val);
// 	float avg();
// 	void reset();
// 	void reset(double t);
// };

struct AccumAvg;
typedef struct AccumAvg AccumAvg;
AccumAvg* AccumAvg_create();
void AccumAvg_destroy(AccumAvg* self);
void AccumAvg_accum(AccumAvg* self, double t, float val);
float AccumAvg_avg(AccumAvg* self);
void AccumAvg_reset(AccumAvg* self);
void AccumAvg_reset2(AccumAvg* self, double t);
	
