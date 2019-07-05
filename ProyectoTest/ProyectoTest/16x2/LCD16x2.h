/*
 * LCD16x2.h
 *
 *  Created on: 12-06-2019
 *      Author: Daniel
 */

#ifndef LCD16X2_H_
#define LCD16X2_H_

#define F_CPU	16000000UL

#include <util/delay.h>
#include <stdio.h>
#include "../i2c/I2C_implement_me.h"

#define LCD16X2_READ	0x7F
#define LCD16X2_WRITE 	0x7E


/*
void data_write(uint8_t data);
void data_read(uint8_t data);*/
void LCD_string(char *s);
void LCD_Write(uint8_t n);
void LCD_Cmd(uint8_t cmd);
void LCD_print(uint8_t LCD_Char);
void LCD_init(void);
void Expander_Write(uint8_t data);

void reverse(char *str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char *res, int afterpoint);

#endif /* LCD16X2_H_ */
