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


// float current_lsb, float r_ohms


#ifndef MAIN_INA226_H_
#define MAIN_INA226_H_

struct INA226Struct;
typedef struct INA226Struct INA226;

#define INA226_SLAVE_ADDRESS_DEFAULT	0x40
#define INA226_CFG_REG		            0x00
#define INA226_SHUNT_VOLT_REG	        0x01
#define INA226_BUS_VOLT_REG	            0x02
#define INA226_POWER_REG	            0x03
#define INA226_CURRENT_REG	            0x04
#define INA226_CAL_REG		            0x05
#define INA226_MASKEN_REG	            0x06
#define INA226_ALERT_LMT_REG	        0x07
#define INA226_MANUFACTURER_ID	        0xFE
#define INA226_DIE_ID		            0xFF

INA226* ina226_create(char* device_file, uint32_t slave_address,
    float current_lsb, float shunt_resistance_ohms);
float ina226_voltage(INA226* ina226);
float ina226_current(INA226* ina226);
float ina226_power(INA226* ina226);

#endif
