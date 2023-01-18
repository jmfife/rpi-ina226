/** INA226 - Simple INA226 Interface
 *  Copyright (C) 2022 John Michael Fife
 *
 *  Code forked from https://github.com/craigpeacock/Linux_I2C 
 *  Jan 15 2022.
 *
 *  Made multi-instance following 
 *  "Test Driven Development for Embedded C" by James W. Grenning,
 *  and other similar Ansi C OOP suggestions.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "i2c.h"
#include "ina226.h"

struct INA226Struct {
	I2C* i2c_p;
	uint32_t slave_address;
	float current_lsb;
};

// Initialization 
void ina226_init(INA226* self_p, char* device_file, uint32_t slave_address,
	float current_lsb, float shunt_resistance_ohms) {
	self_p->i2c_p = i2c_create(device_file);
	self_p->slave_address = slave_address;
	self_p->current_lsb = current_lsb;
	i2c_write_short(self_p->i2c_p, slave_address, INA226_CFG_REG, 0x8000);	// Reset
	i2c_write_short(self_p->i2c_p, slave_address, INA226_CFG_REG, 0x4527);	// Average over 16 Samples
	uint16_t cal = (uint16_t) (0.00512f / current_lsb / shunt_resistance_ohms);
	i2c_write_short(self_p->i2c_p, slave_address, INA226_CAL_REG, cal);

	// printf("Manufacturer ID:        0x%04X\r\n",i2c_read_short(self_p->i2c_p, slave_address, INA226_MANUFACTURER_ID));
	// printf("Die ID Register:        0x%04X\r\n",i2c_read_short(self_p->i2c_p, slave_address, INA226_DIE_ID));
	// printf("Configuration Register: 0x%04X\r\n",i2c_read_short(self_p->i2c_p, slave_address, INA226_CFG_REG));
	// printf("\r\n");
	
	sleep(1);	// let the chip initialize
}

// Allocation + initialization 
INA226* ina226_create(char* device_file, uint32_t slave_address,
	float current_lsb, float shunt_resistance_ohms) {
	INA226* ina226_p = (INA226*)malloc(sizeof(INA226));
	ina226_init(ina226_p, device_file, slave_address, current_lsb, shunt_resistance_ohms);
	return ina226_p;
}

float ina226_voltage(INA226* self_p)
{
	uint16_t iBusVoltage;
	float fBusVoltage;

	iBusVoltage = i2c_read_short(self_p->i2c_p, self_p->slave_address, INA226_BUS_VOLT_REG);
	// Below calculation per data sheet section 7.5.1.
	// Bus voltage register is counts, and voltage LSB is a fixed 1.25 mV/bit.
	fBusVoltage = (iBusVoltage) * 0.00125;
	return (fBusVoltage);
}

float ina226_current(INA226* self_p)
{
	int16_t iCurrent;
	float fCurrent;

	iCurrent = i2c_read_short(self_p->i2c_p, self_p->slave_address, INA226_CURRENT_REG);
	// Below calculation per data sheet section 7.5.1.
	// Current register value = ((ShuntVoltage * CalibrationRegister) / 2048).
	// Must multiply what we get from the current register by the current LSB.
	fCurrent = (float)iCurrent * self_p->current_lsb;
	return (fCurrent);
}

float ina226_power(INA226* self_p)
{
	int16_t iPower;
	float fPower;

	iPower = i2c_read_short(self_p->i2c_p, self_p->slave_address, INA226_POWER_REG);
	// Below calculation per data sheet section 7.5.1.
	// The Power Register LSB is internally programmed to equal 25 times the programmed 
	// value of the Current_LSB
	fPower = (float)iPower * 25 * self_p->current_lsb;
	return (fPower);
}
