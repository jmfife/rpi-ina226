/** I2C - Simple I2C Interface
*   Copyright (C) 2022 John Michael Fife
*/

#ifndef MAIN_I2C_H_
#define MAIN_I2C_H_

#include <stdint.h>

struct I2CStruct;
typedef struct I2CStruct I2C;

I2C* i2c_create(char* devname);
uint16_t i2c_read_short(I2C* self_p, uint8_t address, uint8_t command);
uint32_t i2c_write_short(I2C* self_p, uint8_t address, uint8_t command, uint16_t data);
uint32_t i2c_read_buf(I2C* self_p, uint8_t address, uint8_t command, uint8_t* buffer, uint8_t len);

#endif