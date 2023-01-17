/*
 * I2C - Simple I2C Interface
 * Copyright (C) 2022 John Michael Fife
 *
 * Incomplete code forked from https://github.com/craigpeacock/Linux_I2C 
 * Jan 15 2022.
 *
 * Made multi-instance following 
 * "Test Driven Development for Embedded C" by
 * James W. Grenning
 * and other references about writing OOP with Ansi C.
 *
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