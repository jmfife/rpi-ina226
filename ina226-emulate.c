#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <argp.h>
#include <stdbool.h>
#include <sys/time.h>
#include "AccumAvg.h"
#include <limits.h>

#define INA226_ADDRESS 0x40

int fd;
uint64_t config;
float current_lsb;

const char *argp_program_version = "ina226 1.0";
const char *argp_program_bug_address = "https://github.com/jmfife/rpi-ina226";
static char doc[] = "Use a Raspberry Pi with an INA226 chip to measure DC voltage and current";
static char args_doc[] = "";
static struct argp_option options[] = {
	{ "sph",        's',   "SPH",   0,  "Samples per hour",         			0},
	{ "interval",   'i',   0,       0,  "Interval mode",	   					0},
	{ "iph",        'p',   "IPH",   0,  "Intervals per hour (interval mode)",   0},
	{ "spi",        'k',   "SPI",   0,  "Samples per interval (interval mode)", 0},
	{ "shuntr",     'r',   "SR",    0,  "Shunt resistance (Ohms)",              0},
	{ "maxi",       'm',   "MXI",   0,  "Maximum current (A)",                  0},
	{ 0 }
};

struct arguments {
	int samples_per_hour;
	bool interval_mode;
	int intervals_per_hour;
	int samples_per_interval;
	float shunt_resistance_ohms;
	float max_current_a;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	(void) arg; /* suppress warning about unused parameter */
	struct arguments *arguments = (struct arguments *) state->input;
	switch (key) {
		case 's': arguments->samples_per_hour = atoi(arg); break;
		case 'i': arguments->interval_mode = true; break;
		case 'p': arguments->intervals_per_hour = atoi(arg); break;
		case 'k': arguments->samples_per_interval = atoi(arg); break;
		case 'r': arguments->shunt_resistance_ohms = atof(arg); break;
		case 'm': arguments->max_current_a = atof(arg); break;
		case ARGP_KEY_ARG:
			return 0;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char* argv[]) {
	struct arguments arguments;

	arguments.samples_per_hour = 1800;
	arguments.interval_mode = false;
	arguments.intervals_per_hour = (int) 60;
	arguments.samples_per_interval = (int) 12;
	arguments.shunt_resistance_ohms = 0.0015f;
	arguments.max_current_a = 200.0f;

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

    float voltage, current, power, shunt;
    AccumAvg* voltage_avg = AccumAvg_create();
    AccumAvg* current_avg = AccumAvg_create();
    AccumAvg* power_avg = AccumAvg_create();
	int firstinterval = true;

	struct timeval rawtimeval;
	double rawtimeval_sec;
	double rawtimeval_intervalstart_sec;
	double seconds_per_sample;
	double seconds_to_next_sample;
	long int subinterval, current_subinterval;
	long subinterval3;
	int subintervali;
	char datastring[1000];
	char datastring_interval[1000];
	long int maxlong = LONG_MAX;

	/* set up timer */
	if (arguments.interval_mode) {
		seconds_per_sample = 3600.0 / arguments.intervals_per_hour / arguments.samples_per_interval;
	} else {
		seconds_per_sample = 3600.0 / arguments.samples_per_hour;
	}
	if (seconds_per_sample < 1.0) {
		seconds_per_sample = 1.0;
	}
	// printf("seconds_per_sample = %f\n", seconds_per_sample);
	current_subinterval = 0; 	// starting subinterval just needs to be != the true current interval

    for (;;) {
        //ina226_configure(INA226_TIME_8MS, INA226_TIME_8MS, INA226_AVERAGES_16, INA226_MODE_SHUNT_BUS_TRIGGERED);
        //ina226_wait();

        gettimeofday(&rawtimeval, NULL);
        rawtimeval_sec = (double)rawtimeval.tv_sec + (double)rawtimeval.tv_usec / 1e6;
        // doing this a little sketchy - should really used fixed-width types here
        subinterval = (unsigned long int) (rawtimeval_sec / seconds_per_sample);
        if (subinterval != current_subinterval) {
            current_subinterval = subinterval;
            seconds_to_next_sample = seconds_per_sample - fmod(rawtimeval_sec, seconds_per_sample);
            usleep(seconds_to_next_sample * 1e6);
            voltage = 12.0;
            current = 1.0;
            power = 12.0;
            shunt = 9.055;
            gettimeofday(&rawtimeval, NULL);
            rawtimeval_sec = (double)rawtimeval.tv_sec + (double)rawtimeval.tv_usec / 1e6;
            sprintf(datastring, "\"V\": %.3f, \"I\": %.3f, \"P\": %.1f", voltage, current, power);
            if (arguments.interval_mode) {
                //printf("{\"ts\": %.3f, %s}\n", rawtimeval_sec, datastring);
                // Handle Interval
                AccumAvg_accum(voltage_avg, rawtimeval_sec, voltage);
                AccumAvg_accum(current_avg, rawtimeval_sec, current);
                AccumAvg_accum(power_avg, rawtimeval_sec, power);
                if ((current_subinterval + 1) % arguments.samples_per_interval == 0) {
                    //printf("{\"ts\": %.3f, %s}\n", rawtimeval_sec, datastring);
                    if (!firstinterval) {
                        sprintf(datastring_interval, "\"V\": %.3f, \"I\": %.3f, \"P\": %.1f",
                            AccumAvg_avg(voltage_avg), AccumAvg_avg(current_avg), AccumAvg_avg(power_avg));
                        //printf("{\"ts\": %.3f, \"interval_duration\": %.3f, \"data\": %s}\n", \
						//	rawtimeval_sec, rawtimeval_sec - rawtimeval_intervalstart_sec, datastring_interval);
                        printf("{\"ts\": %.3f, %s}\n", \
                            rawtimeval_sec, datastring_interval);
                    }
                    fflush(NULL);
                    AccumAvg_reset2(voltage_avg, rawtimeval_sec);
                    AccumAvg_reset2(current_avg, rawtimeval_sec);
                    AccumAvg_reset2(power_avg, rawtimeval_sec);
                    rawtimeval_intervalstart_sec = rawtimeval_sec;
                    firstinterval = false;
                }
                fflush(NULL);
            }
            else {

                printf("{\"ts\": %.3f, %s}\n", rawtimeval_sec, datastring);
                fflush(NULL);
            }
        }
    }

	return 0;
}

