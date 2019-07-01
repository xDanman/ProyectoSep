/*
 * bmp180.h
 *
 * Created: 30-06-2019 16:56:58
 *  Author: Daniel
 */ 


#ifndef BMP180_H_
#define BMP180_H_

#define F_CPU	16000000UL

#include <util/delay.h>
#include <math.h>
#include <stdbool.h>
#include "../i2c/I2C_implement_me.h"

#define bmp180_READ		0xEF
#define bmp180_WRITE 	0xEE

uint16_t read_register(uint8_t reg);
void write_register(uint8_t reg, uint8_t word);
long read_pressure(uint8_t oss);
void begin_bmp180(void);
long getTemperature(void);
long getPressure(uint8_t oss);
long altitude(long P);

#endif /* BMP180_H_ */