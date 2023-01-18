/** INA226 - Simple INA226 Interface
 *  Copyright (C) 2022 John Michael Fife
 **/


#ifndef MAIN_INA226_H_
#define MAIN_INA226_H_

#include <stdint.h>

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
