/*
 * INA226 - TI Current/Voltage/Power Monitor Code
 * Copyright (C) 2021 Craig Peacock
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
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
};

// Initialization 
void ina226_init(INA226* self, char* device_file, uint32_t slave_address,
	float current_lsb, float shunt_resistance_ohms) {
	self->i2c_p = i2c_create(device_file);
	i2c_write_short(self->i2c_p, slave_address, INA226_CFG_REG, 0x8000);	// Reset
	i2c_write_short(self->i2c_p, slave_address, INA226_CFG_REG, 0x4527);	// Average over 16 Samples
	uint16_t cal = (uint16_t) (0.00512f / current_lsb / shunt_resistance_ohms);
	i2c_write_short(self->i2c_p, slave_address, INA226_CAL_REG, cal);	// 1A, 0.100Ohm Resistor

	// printf("Manufacturer ID:        0x%04X\r\n",i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_MANUFACTURER_ID));
	// printf("Die ID Register:        0x%04X\r\n",i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_DIE_ID));
	// printf("Configuration Register: 0x%04X\r\n",i2c_read_short(i2c_master_port, INA226_SLAVE_ADDRESS, INA226_CFG_REG));
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

float ina226_voltage(INA226* self)
{
	uint16_t iBusVoltage;
	float fBusVoltage;

	iBusVoltage = i2c_read_short(self->i2c_p, self->slave_address, INA226_BUS_VOLT_REG);
	//printf("iBusVoltage = %04x\r\n", iBusVoltage);
	fBusVoltage = (iBusVoltage) * 0.00125;
	//printf("Bus Voltage = %.2fV, ", fBusVoltage);

	return (fBusVoltage);
}

float ina226_current(INA226* self)
{
	int16_t iCurrent;
	float fCurrent;

	iCurrent = i2c_read_short(self->i2c_p, self->slave_address, INA226_CURRENT_REG);
	// Internally Calculated as Current = ((ShuntVoltage * CalibrationRegister) / 2048)
	fCurrent = iCurrent * 0.0005;
	//printf("Current = %.3fA\r\n", fCurrent);

	return (fCurrent);
}

float ina226_power(INA226* self)
{
	int16_t iPower;
	float fPower;

	iPower = i2c_read_short(self->i2c_p, self->slave_address, INA226_POWER_REG);
	// The Power Register LSB is internally programmed to equal 25 times the programmed value of the Current_LSB
	fPower = iPower * 0.0125;

	//printf("Power = %.2fW\r\n", fPower);
	return (fPower);
}
