#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <argp.h>
#include <stdbool.h>
#include "ina226.h"
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
	{ "emulate",    'e',   0,       0,  "Emulation mode",  			  			0},
	{ "sph",        's',   "SPH",   0,  "Samples per hour",         			0},
	{ "interval",   'i',   0,       0,  "Interval mode (default)",	   			0},
	{ "iph",        'p',   "IPH",   0,  "Intervals per hour (interval mode)",   0},
	{ "spi",        'k',   "SPI",   0,  "Samples per interval (interval mode)", 0},
	{ "shuntr",     'r',   "SR",    0,  "Shunt resistance (Ohms)",              0},
	{ "maxi",       'm',   "MXI",   0,  "Maximum current (A)",                  0},
	{ 0 }
};

struct arguments {
	bool emulate_mode;
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
		case 'e': arguments->emulate_mode = true; break;
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

uint16_t read16(int fd, uint8_t ad){
	uint16_t result = wiringPiI2CReadReg16(fd,ad);
	// Chip uses different endian
	return  (result<<8) | (result>>8);
}

void write16(int fd, uint8_t ad, uint16_t value) {
	// Chip uses different endian
	wiringPiI2CWriteReg16(fd,ad,(value<<8)|(value>>8));
}

// R of shunt resistor in ohm. Max current in Amp
void ina226_calibrate(float r_shunt, float max_current) {
	current_lsb = max_current / (1 << 15);
	float calib = 0.00512 / (current_lsb * r_shunt);
	uint16_t calib_reg = (uint16_t) floorf(calib);
	current_lsb = 0.00512 / (r_shunt * calib_reg);

	//printf("LSB %f\n",current_lsb);
	//printf("Calib %f\n",calib);
	//printf("Calib R%#06x / %d\n",calib_reg,calib_reg);

	write16(fd,INA226_REG_CALIBRATION, calib_reg);
}

void ina226_configure(uint8_t bus, uint8_t shunt, uint8_t average, uint8_t mode) {
	config = (average<<9) | (bus<<6) | (shunt<<3) | mode;
	write16(fd,INA226_REG_CONFIGURATION, config);
}

uint16_t ina226_conversion_ready() {
	return read16(fd,INA226_REG_MASK_ENABLE) & INA226_MASK_ENABLE_CVRF;
}

void ina226_wait() {
	uint8_t average = (config>>9) & 7;
	uint8_t bus = (config>>6) & 7;
	uint8_t shunt = (config>>3) & 7;

	uint32_t total_wait = (wait[bus] + wait[shunt] + (average ? avgwaits[bus>shunt ? bus : shunt] : 0)) * averages[average];

	usleep(total_wait+1000);

	int count=0;
	while(!ina226_conversion_ready()){
		count++;
	}
	//printf("%d\n",count);
}

void ina226_read(float *voltage, float *current, float *power, float* shunt_voltage) {
	if (voltage) {
		uint16_t voltage_reg = read16(fd, INA226_REG_BUS_VOLTAGE);
		*voltage = (float) voltage_reg * 1.25e-3;
	}

	if (current) {
		int16_t current_reg = (int16_t) read16(fd, INA226_REG_CURRENT);
		*current = (float) current_reg * current_lsb;
	}

	if (power) {
		int16_t power_reg = (int16_t) read16(fd, INA226_REG_POWER);
		*power = (float) power_reg * 25.0 * current_lsb;
	}

	if (shunt_voltage) {
		int16_t shunt_voltage_reg = (int16_t) read16(fd, INA226_REG_SHUNT_VOLTAGE);
		*shunt_voltage = (float) shunt_voltage_reg * 2.5e-3;
	}
}

inline void ina226_reset() {
	write16(fd, INA226_REG_CONFIGURATION, config = INA226_RESET);
}

inline void ina226_disable() {
	write16(fd, INA226_REG_CONFIGURATION, config = INA226_MODE_OFF);
}

int main(int argc, char *argv[]) {
	struct arguments arguments;

	arguments.emulate_mode = false;
	arguments.samples_per_hour = 1800;
	arguments.interval_mode = true;
	arguments.intervals_per_hour = (int) 4;
	arguments.samples_per_interval = (int) 450;
	arguments.shunt_resistance_ohms = 0.00155f;
	arguments.max_current_a = 200.0f;

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	float voltage, current, power, shunt;
	AccumAvg voltage_avg, current_avg, power_avg;
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
	if(!(arguments.emulate_mode)) {
		fd = wiringPiI2CSetup(INA226_ADDRESS);
		if(fd < 0) {
			printf("Device not found");
			return -1;
		}
	}

	//printf("Manufacturer 0x%X Chip 0x%X\n",read16(fd,INA226_REG_MANUFACTURER),read16(fd,INA226_REG_DIE_ID));

	ina226_calibrate(arguments.shunt_resistance_ohms, arguments.max_current_a);
	usleep(50000); // wait 50ms for it to take

	// BUS / SHUNT / Averages / Mode
	ina226_configure(INA226_TIME_8MS, INA226_TIME_8MS, INA226_AVERAGES_16, INA226_MODE_SHUNT_BUS_CONTINUOUS);

	for(;;) {
		//ina226_configure(INA226_TIME_8MS, INA226_TIME_8MS, INA226_AVERAGES_16, INA226_MODE_SHUNT_BUS_TRIGGERED);
		//ina226_wait();

			gettimeofday(&rawtimeval, NULL);
			rawtimeval_sec = (double) rawtimeval.tv_sec + (double) rawtimeval.tv_usec / 1e6;
			// doing this a little sketchy - should really used fixed-width types here
			subinterval = (unsigned long int) (rawtimeval_sec / seconds_per_sample);
			if (subinterval != current_subinterval) {
				current_subinterval = subinterval;
				seconds_to_next_sample = seconds_per_sample - fmod(rawtimeval_sec, seconds_per_sample);
				usleep(seconds_to_next_sample*1e6);
			if (!(arguments.emulate_mode)) {
				ina226_read(&voltage, &current, NULL, NULL);
				power = current*voltage;		// use this because the INA226 does not represent power as a signed value
			} else {
				voltage = 12.0;
				current = 1.0;
				power = 12.0;
				shunt = 9.055;
			}
			gettimeofday(&rawtimeval, NULL);
			rawtimeval_sec = (double) rawtimeval.tv_sec + (double) rawtimeval.tv_usec / 1e6;
			sprintf(datastring, "{\"V\": %.3f, \"I\": %.3f, \"P\": %.1f}", voltage, current, power);
			if (arguments.interval_mode) {
				printf("{\"ts\": %.3f, \"table\": \"now\", \"data\": %s}\n", rawtimeval_sec, datastring);
				// Handle Interval
				voltage_avg.accum(rawtimeval_sec, voltage);
				current_avg.accum(rawtimeval_sec, current);
				power_avg.accum(rawtimeval_sec, power);
				if ((current_subinterval + 1) % arguments.samples_per_interval == 0) {
					printf("{\"ts\": %.3f, \"table\": \"instant\", \"data\": %s}\n", rawtimeval_sec, datastring);
					if (!firstinterval) {
						sprintf(datastring_interval, "{\"V\": %.3f, \"I\": %.3f, \"P\": %.1f}", 
							voltage_avg.avg(), current_avg.avg(), power_avg.avg());
						printf("{\"ts\": %.3f, \"table\": \"interval\", \"interval_duration\": %.3f, \"data\": %s}\n", \
							rawtimeval_sec, rawtimeval_sec - rawtimeval_intervalstart_sec, datastring_interval);
					}
					fflush(NULL);
					voltage_avg.reset(rawtimeval_sec);
					current_avg.reset(rawtimeval_sec);
					power_avg.reset(rawtimeval_sec);
					rawtimeval_intervalstart_sec = rawtimeval_sec;
					firstinterval = false;
				}
				fflush(NULL);
			} else {
				printf("{\"ts\": %.3f, \"data\": %s}\n", rawtimeval_sec, datastring);
				fflush(NULL);
			}
		}
	}

	ina226_disable();

	return 0;
}

