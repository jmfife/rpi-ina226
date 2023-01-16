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
// #include "i2c.h"
#include "ina226.h"

int fd;

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


// R of shunt resistor in ohm. Max current in Amp
// void ina226_calibrate(float r_shunt, float max_current) {
// 	current_lsb = max_current / (1 << 15);
// 	float calib = 0.00512 / (current_lsb * r_shunt);
// 	uint16_t calib_reg = (uint16_t) floorf(calib);
// 	current_lsb = 0.00512 / (r_shunt * calib_reg);

// 	//printf("LSB %f\n",current_lsb);
// 	//printf("Calib %f\n",calib);
// 	//printf("Calib R%#06x / %d\n",calib_reg,calib_reg);

// 	write16(fd,INA226_REG_CALIBRATION, calib_reg);
// }

int main(int argc, char *argv[]) {
	struct arguments arguments;
	arguments.device_file = "/dev/i2c-1";
	arguments.current_lsb = 0.001f;				// default = 1 mA/bit
	arguments.slave_address = 0x40;				// default address for INA226
	arguments.shunt_resistance_ohms = 0.0015f;	// default 1.5 mOhms
	arguments.emulate_mode = false;
	arguments.samples_per_hour = 1800;
	arguments.interval_mode = false;
	arguments.intervals_per_hour = (int) 60;
	arguments.samples_per_interval = (int) 12;	

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	float voltage, current, power;
    AccumAvg* voltage_avg = AccumAvg_create();
    AccumAvg* current_avg = AccumAvg_create();
    AccumAvg* power_avg = AccumAvg_create();
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
		// // printf("Initializing I2C device at addr 0x%02X on %s \r\n", i2caddr, devname);
		// hI2C = i2c_init(devname);
		// // printf("Initalizing INA226.\r\n");
		// i2caddr += 1; // JUST TO STOP WARNING
		// ina226_init(hI2C);
		ina226_p = ina226_create(arguments.device_file, arguments.slave_address,
			arguments.current_lsb, arguments.shunt_resistance_ohms);
		// printf("INA226 Initialized.\r\n");
	}

	// printf("Interval Mode: %d\r\n", arguments.interval_mode);

	for (;;) {
		//ina226_configure(INA226_TIME_8MS, INA226_TIME_8MS, INA226_AVERAGES_16, INA226_MODE_SHUNT_BUS_TRIGGERED);
		//ina226_wait();

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
				// power = current * voltage;		// use this because the INA226 does not represent power as a signed value
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
				//printf("{\"time\": %.3f, %s}\n", rawtimeval_sec, datastring);
				// Handle Interval
                AccumAvg_accum(voltage_avg, rawtimeval_sec, voltage);
                AccumAvg_accum(current_avg, rawtimeval_sec, current);
                AccumAvg_accum(power_avg, rawtimeval_sec, power);
				if ((current_subinterval + 1) % arguments.samples_per_interval == 0) {
					if (!firstinterval) {
						sprintf(datastring_interval, "\"V\": %.3f, \"I\": %.3f, \"P\": %.1f",
							AccumAvg_avg(voltage_avg), AccumAvg_avg(current_avg), AccumAvg_avg(power_avg));
                        printf("{\"time\": %lu, \"fields\": {%s}}\n", \
                            (unsigned long) (rawtimeval_sec*1e9), datastring_interval);
					}
					fflush(NULL);
					AccumAvg_reset2(voltage_avg, rawtimeval_sec);
					AccumAvg_reset2(current_avg, rawtimeval_sec);
					AccumAvg_reset2(power_avg, rawtimeval_sec);
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

