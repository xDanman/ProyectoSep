#include <avr/io.h>

#include "LCD16x2.h"

int Rs;

void LCD_string(char *s){
    while(*s){
        LCD_print(*s);//Address sube solo
        s++;
    }
}
	
	
	void LCD_Write(uint8_t n) {
		n |= Rs;
		Expander_Write(n);
		Expander_Write(n | 0x04); //Enable
		_delay_us(2);
		Expander_Write(n & 0xFB); //Not enable
		_delay_ms(2);
	}
	
	void LCD_Cmd(uint8_t cmd) {
		Rs = 0;
		LCD_Write(cmd & 0xF0);
		LCD_Write((cmd << 4) & 0xF0);
	}
	
	void LCD_print(uint8_t LCD_Char){
		Rs = 1;
		LCD_Write(LCD_Char & 0xF0);
		LCD_Write((LCD_Char << 4) & 0xF0);
	}
	
	void LCD_init(void) {
		Expander_Write(0);
		_delay_ms(50);
		
		Rs = 0;
		LCD_Write(0x30);
		_delay_ms(3);
		LCD_Write(0x30);
		LCD_Write(0x30);
		
		LCD_Write(0x20);
		
		LCD_Cmd(0x28);
		_delay_ms(3);
		LCD_Cmd(0x08);
		_delay_ms(3);
		LCD_Cmd(0x01);
		_delay_ms(3);
		LCD_Cmd(0x06);
		_delay_ms(3);
		LCD_Cmd(0x0C);
		_delay_ms(3); 
	}
	
	void Expander_Write(uint8_t data) {
		i2cSendStart();							// send start condition
		i2cWaitForComplete();
		i2cSendByte(LCD16X2_WRITE); 				// send WRITE address of TMP102
		i2cWaitForComplete();
		i2cSendByte( data ); 						// set TMP102 pointer register to 0 (read temperature)
		i2cWaitForComplete();
		i2cSendStop();
	}
	
	//Pasar de float a string
	
	
	void reverse(char *str, int len)
	{
		int i=0, j=len-1, temp;
		while (i<j)
		{
			temp = str[i];
			str[i] = str[j];
			str[j] = temp;
			i++; j--;
		}
	}

	// Converts a given integer x to string str[].  d is the number
	// of digits required in output. If d is more than the number
	// of digits in x, then 0s are added at the beginning.
	int intToStr(int x, char str[], int d)
	{
		int i = 0;
		while (x)
		{
			str[i++] = (x%10) + '0';
			x = x/10;
		}
		
		// If number of digits required is more, then
		// add 0s at the beginning
		while (i < d)
		str[i++] = '0';
		
		reverse(str, i);
		str[i] = '\0';
		return i;
	}

	// Converts a floating point number to string.
	void ftoa(float n, char *res, int afterpoint)
	{
		// Extract integer part
		int ipart = (int)n;
		
		// Extract floating part
		float fpart = n - (float)ipart;
		
		// convert integer part to string
		int i = intToStr(ipart, res, 0);
		
		// check for display option after point
		if (afterpoint != 0)
		{
			res[i] = '.';  // add dot
			
			// Get the value of fraction part upto given no.
			// of points after dot. The third parameter is needed
			// to handle cases like 233.007
			fpart = fpart * pow(10, afterpoint);
			
			intToStr((int)fpart, res + i + 1, afterpoint);
		}
	}
