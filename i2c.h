/*
 * I2C - Simple I2C Interface
 * Copyright (C) 2022 John Michael Fife
 *
 * Incomplete code forked from https://github.com/craigpeacock/Linux_I2C 
 * Jan 15 2022.
 *
 * Made ]multi-instance following 
 * "Test Driven Development for Embedded C" by
 * James W. Grenning.
 *
 */

#ifndef MAIN_I2C_H_
#define MAIN_I2C_H_

#include <stdint.h>

struct I2CDeviceStruct;
typedef struct I2CDeviceStruct I2CDevice;

I2CDevice* I2CDevice_create(char* devname, float current_lsb, float r);
uint16_t i2c_read_short(I2CDevice* device_p, uint8_t address, uint8_t command);
uint32_t i2c_write_short(I2CDevice* device_p, uint8_t address, uint8_t command, uint16_t data);


// uint32_t i2c_init(char *devname);

// uint8_t i2c_read_byte(uint32_t i2c_master_port, uint8_t address, uint8_t command);
// uint32_t i2c_write_byte(uint32_t i2c_master_port, uint8_t address, uint8_t command, uint8_t data);

// uint16_t i2c_read_short(uint32_t i2c_master_port, uint8_t address, uint8_t command);
// uint32_t i2c_write_short(uint32_t i2c_master_port, uint8_t address, uint8_t command, uint16_t data);

// uint32_t i2c_read_buf(uint32_t i2c_master_port, uint8_t address, uint8_t command, uint8_t *buffer, uint8_t len);
// uint32_t i2c_write_buf(uint32_t i2c_master_port, uint8_t address, uint8_t command, uint8_t *data, uint8_t len);

#endif