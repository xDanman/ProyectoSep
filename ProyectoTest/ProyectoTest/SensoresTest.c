/*
 * ProyectoTest.c
 *
 * Created: 26-06-2019 14:32:13
 * Author : Daniel
 */ 

// read and write addresses; set ADD0 = GND

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "i2c/I2C_implement_me.h"
#include "16x2/LCD16x2.h"
#include "bmp180/bmp180.h"
#include "USART/USART_implement_me.h"

void setup_adc(void);
int valorAdc(void);

int main(void)
{
	    // I2C initialiZation
	TWBR = 12;									// 400 kHz at 16MHz crystal and no prescaler
	TWSR &= ~((1 << TWPS1) | (1 << TWPS0));		// prescaler 0
	TWDR = 0xFF;								// sending only ones equals an idle high SDA line
	TWCR = (1<<TWEN)|							// Enable TWI-interface and release TWI pins. 
	(0<<TWIE)|(0<<TWINT)|				// Disable Interrupt.
	(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|	// No Signal requests.
	(0<<TWWC);
		
	long p;
	long altura;
	
	LCD_init();
	setup_adc();
	
	LCD_string("Hello");
	LCD_Cmd(0xC0);
	LCD_string("Madafackas");
	
	struct USART_configuration config_57600_8N1 = {57600, 8, 'N', 1};
	USART_Init(config_57600_8N1);
	
	begin_bmp180();
	p = getPressure( 0 );
	
	p = getPressure( 1 );
	
	p = getPressure( 2 );
	
	p = getPressure( 3 );
    while (1) 
    {
		int rain = valorAdc();
		char buffer[10];
		itoa(rain, buffer, 10);
		if(valorAdc()<300){
			USART_Transmit_String(buffer);
			USART_Transmit_String(" Heavy rain\r\n");
		}
		else if(valorAdc()<500){
			USART_Transmit_String(buffer);
			USART_Transmit_String(" Moderate rain\r\n");
		}
		else if(valorAdc()<800){
			USART_Transmit_String(buffer);
			USART_Transmit_String(" Few drops\r\n");
		}
		else{
			USART_Transmit_String(buffer);
			USART_Transmit_String(" No rain\r\n");
		}
		_delay_ms(500);
    }
}



void setup_adc(){
	ADMUX |= (1<<REFS0) | (0<<MUX0);// AVcc referencia, ADC0
	
	ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);//ADC on,	auto trigger enable, interrupt, /128
	ADCSRA |= (1<<ADEN);
	DIDR0 |= (1<<ADC0D);
	
	ADCSRA |= (1<<ADSC);

}
int valorAdc(){
	ADCSRA |= (1<<ADSC);                //Starts a new conversion
	while(ADCSRA & (1<<ADSC));            //Wait until the conversion is done
	return ADC;
}