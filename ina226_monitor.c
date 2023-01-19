/** INA226 Monitor
*	Copyright (C) 2022 John Michael Fife
*
*	Monitor INA226 voltage, current, and power measurements,
*	and write to stdout.
*
*	For Linux OS only.
*/

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
#include <limits.h>
#include "ina226.h"
#include "accum_mean.h"


const char *argp_program_version = "ina226 2.0";
const char *argp_program_bug_address = "https://github.com/jmfife/rpi-ina226";
static char doc[] = "Interface to INA226 chip on RPi to measure DC voltage and current";
static char args_doc[] = "";
static struct argp_option options[] = {
	{ "dev",		'd',	"DEV",	0,	"Device file, e.g. /dev/i2c-1",			0},
	{ "addr",		'a',	"ADDR",	0,	"I2C Slave Address",					0},
	{ "shuntr",     'r',	"SR",	0,  "Shunt resistance (Ohms)",				0},
	{ "clsb",       'c',	"CLSB",	0,  "Current LSB (A/bit)",					0},
	{ "emulate",    'e',	0,		0,  "Emulation mode",  			  			0},
	{ "sph",        's',	"SPH",	0,  "Samples per hour",         			0},
	{ "interval",   'i',	0,		0,  "Interval mode",	   					0},
	{ "iph",        'p',	"IPH",	0,  "Intervals per hour (interval mode)",	0},
	{ "spi",        'k',	"SPI",	0,  "Samples per interval (interval mode)",	0},
	{ 0 }
};

struct arguments {
	char* device_file;
	uint8_t slave_address;
	float current_lsb;
	float shunt_resistance_ohms;
	bool emulate_mode;
	int samples_per_hour;
	bool interval_mode;
	int intervals_per_hour;
	int samples_per_interval;

};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
	(void)arg; /* suppress warning about unused parameter */
	struct arguments* arguments = (struct arguments*)state->input;
	switch (key) {
	case 'd': arguments->device_file = arg; break;
	case 'a': arguments->slave_address = strtol(arg, NULL, 16); break;
	case 'c': arguments->current_lsb = atof(arg); break;
	case 'r': arguments->shunt_resistance_ohms = atof(arg); break;
	case 'e': arguments->emulate_mode = true; break;
	case 's': arguments->samples_per_hour = atoi(arg); break;
	case 'i': arguments->interval_mode = true; break;
	case 'p': arguments->intervals_per_hour = atoi(arg); break;
	case 'k': arguments->samples_per_interval = atoi(arg); break;
	case ARGP_KEY_ARG:
		return 0;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char *argv[]) {
	struct arguments arguments;

	// DEFAULTS
	arguments.device_file = "/dev/i2c-1";
	// for INA226 settings, see INA226 data sheet
	arguments.current_lsb = 0.002f;					// default = 2 mA/bit -> max current 65.54A
	arguments.slave_address = 0x40;					// default address for INA226
	arguments.shunt_resistance_ohms = 0.009055f;	// from shunt calibration on 2020-04-19
	arguments.emulate_mode = false;
	arguments.samples_per_hour = 360;
	arguments.interval_mode = false;
	arguments.intervals_per_hour = (int) 60;
	arguments.samples_per_interval = (int) 12;	

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	float voltage, current, power;
    AccumMean* voltage_mean = accum_mean_create();
    AccumMean* current_mean = accum_mean_create();
    AccumMean* power_mean = accum_mean_create();
	int firstinterval = true;

	struct timeval rawtimeval;
	double rawtimeval_sec;
	double seconds_per_sample;
	double seconds_to_next_sample;
	long int subinterval, current_subinterval;
	char datastring[1000];
	char datastring_interval[1000];
	INA226* ina226_p;

	time_t t;
	srand((unsigned)time(&t));
		
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
	if(!(arguments.emulate_mode)) {
		ina226_p = ina226_create(arguments.device_file, arguments.slave_address,
			arguments.current_lsb, arguments.shunt_resistance_ohms);
	}

	for (;;) {
		gettimeofday(&rawtimeval, NULL);
		rawtimeval_sec = (double)rawtimeval.tv_sec + (double)rawtimeval.tv_usec / 1.0e6;
		// doing this a little sketchy - should really used fixed-width types here
		subinterval = (unsigned long int) (rawtimeval_sec / seconds_per_sample);
		if (subinterval != current_subinterval) {
			current_subinterval = subinterval;
			seconds_to_next_sample = seconds_per_sample - fmod(rawtimeval_sec, seconds_per_sample);
			usleep(seconds_to_next_sample * 1e6);
			if (!(arguments.emulate_mode)) {
				voltage = ina226_voltage(ina226_p);
				current = ina226_current(ina226_p);
				power = ina226_power(ina226_p);
				// next line because INA226 returns abs(power)
				if (current < 0) power = -power;
			}
			else {
				voltage = 12.0 + (float) rand() / RAND_MAX - 0.5;
				current = 3.0 + ((float) rand() / RAND_MAX - 0.5)*0.1;
				power = voltage*current;
			}
			gettimeofday(&rawtimeval, NULL);
			rawtimeval_sec = (double)rawtimeval.tv_sec + (double)rawtimeval.tv_usec / 1e6;
			sprintf(datastring, "\"V\": %.3f, \"I\": %.3f, \"P\": %.1f", voltage, current, power);
			if (arguments.interval_mode) {
				// Handle Interval
                accum_mean_accum(voltage_mean, rawtimeval_sec, voltage);
                accum_mean_accum(current_mean, rawtimeval_sec, current);
                accum_mean_accum(power_mean, rawtimeval_sec, power);
				if ((current_subinterval + 1) % arguments.samples_per_interval == 0) {
					if (!firstinterval) {
						sprintf(datastring_interval, "\"V\": %.3f, \"I\": %.3f, \"P\": %.1f",
							accum_mean_yield(voltage_mean), accum_mean_yield(current_mean), accum_mean_yield(power_mean));
                        printf("{\"time\": %lu, \"fields\": {%s}}\n", \
                            (unsigned long) (rawtimeval_sec*1e9), datastring_interval);
					}
					fflush(NULL);
					accum_mean_reset(voltage_mean);
					accum_mean_reset(current_mean);
					accum_mean_reset(power_mean);
					firstinterval = false;
				}
				fflush(NULL);
			}
			else {
					printf("{\"time\": %lu, \"fields\": {%s}}\n", (unsigned long) (rawtimeval_sec*1e9), datastring);
					fflush(NULL);
			}
		}
	}

	// ina226_disable();

	return 0;
}

