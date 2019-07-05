/*
 * bmp180.c
 *
 * Created: 30-06-2019 16:56:43
 *  Author: Daniel
 */ 
#include <avr/io.h>

#include "bmp180.h"

int16_t AC1;
int16_t AC2;
int16_t AC3;
uint16_t AC4;
uint16_t AC5;
uint16_t AC6;
int16_t B1;
int16_t B2;
int16_t MB;
int16_t MC;
int16_t MD;

uint16_t read_register(uint8_t reg) {
	
	uint8_t msb;
	uint8_t lsb;
	uint16_t out;
	
	i2cSendStart();							// send start condition
	i2cWaitForComplete();
	i2cSendByte(bmp180_WRITE); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cSendByte( reg ); 						// set TMP102 pointer register to 0 (read temperature)
	i2cWaitForComplete();
	
	i2cSendStart();							//Repeat start
	i2cWaitForComplete();
	i2cSendByte(bmp180_READ); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cReceiveByte(false);					// receives one byte from the bus and ACKs it//Lo recibido es el registro de lectura
	i2cWaitForComplete();
	msb = i2cGetReceivedByte(); 			// reads the MSB (it is a 12 bit value!)
	i2cWaitForComplete();
	i2cSendStop();
	
	i2cSendStart();							// send start condition
	i2cWaitForComplete();
	i2cSendByte(bmp180_WRITE); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cSendByte( reg+1 ); 						// set TMP102 pointer register to 0 (read temperature)
	i2cWaitForComplete();
		
	i2cSendStart();							//Repeat start
	i2cWaitForComplete();
	i2cSendByte(bmp180_READ); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cReceiveByte(false);					// receives one byte from the bus and ACKs it//Lo recibido es el registro de lectura
	i2cWaitForComplete();
	lsb = i2cGetReceivedByte(); 			// reads the MSB (it is a 12 bit value!)
	i2cWaitForComplete();
	i2cSendStop();
	out = (msb<<8) + lsb;
	return out;
	
}

void write_register(uint8_t reg, uint8_t word) {

	i2cSendStart();							// send start condition
	i2cWaitForComplete();
	i2cSendByte(bmp180_WRITE); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cSendByte( reg ); 						// set TMP102 pointer register to 0 (read temperature)
	i2cWaitForComplete();
	i2cSendByte( word); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cSendStop();
	
}

long read_pressure(uint8_t oss) {
	
	long msb;
	uint16_t lsb;
	uint8_t xlsb;
	long out;
	
	i2cSendStart();							// send start condition
	i2cWaitForComplete();
	i2cSendByte(bmp180_WRITE); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cSendByte( 0xF6 ); 						// set TMP102 pointer register to 0 (read temperature)
	i2cWaitForComplete();
	i2cSendStart();							//Repeat start
	i2cWaitForComplete();
	i2cSendByte(bmp180_READ); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cReceiveByte(false);					// receives one byte from the bus and ACKs it//Lo recibido es el registro de lectura
	i2cWaitForComplete();
	msb = i2cGetReceivedByte(); 			// reads the MSB (it is a 12 bit value!)
	i2cWaitForComplete();
	
	i2cSendStart();							// send start condition
	i2cWaitForComplete();
	i2cSendByte(bmp180_WRITE); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cSendByte( 0xF7 ); 						// set TMP102 pointer register to 0 (read temperature)
	i2cWaitForComplete();
	i2cSendStart();							//Repeat start
	i2cWaitForComplete();
	i2cSendByte(bmp180_READ); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cReceiveByte(false);					// receives one byte from the bus and ACKs it//Lo recibido es el registro de lectura
	i2cWaitForComplete();
	lsb = i2cGetReceivedByte(); 			// reads the MSB (it is a 12 bit value!)
	i2cWaitForComplete();
	
	i2cSendStart();							// send start condition
	i2cWaitForComplete();
	i2cSendByte(bmp180_WRITE); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cSendByte( 0xF8 ); 						// set TMP102 pointer register to 0 (read temperature)
	i2cWaitForComplete();
	i2cSendStart();							//Repeat start
	i2cWaitForComplete();
	i2cSendByte(bmp180_READ); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cReceiveByte(false);					// receives one byte from the bus and ACKs it//Lo recibido es el registro de lectura
	i2cWaitForComplete();
	xlsb = i2cGetReceivedByte(); 			// reads the MSB (it is a 12 bit value!)
	i2cWaitForComplete();

	i2cSendStop();
	
	out = ( (msb<<16) + (lsb<<8) + xlsb)>>(8-oss);
	return out;
	
}

void begin_bmp180(void){
	
	AC1 = read_register(0xAA);	_delay_us(500);
	AC2 = read_register(0xAC);	_delay_us(500);
	AC3 = read_register(0xAE);	_delay_us(500);
	AC4 = read_register(0xB0);	_delay_us(500);
	AC5 = read_register(0xB2); _delay_us(500);
	AC6 = read_register(0xB4); _delay_us(500);
	B1 = read_register(0xB6); _delay_us(500);
	B2 = read_register(0xB8); _delay_us(500);
	MB = read_register(0xBA); _delay_us(500);
	MC = read_register(0xBC); _delay_us(500);
	MD = read_register(0xBE); _delay_us(500);
}

long getTemperature(void){
	write_register( 0xF4, 0x2E);
	_delay_us(4550);
	long UT = read_register(0xF6);
	return UT;
}

long getPressure(uint8_t oss){
	long X1;
	long X2;
	long B3;
	unsigned long B4;
	long B6;
	unsigned long B7;
	long X3;
	long p;
	
	long UT = getTemperature();
	
	write_register( 0xF4, (0x34 + (oss<<6)));
	if(oss == 0){
		_delay_us(4500);
	}
	else if(oss == 1){
		_delay_us(7500);
	}
	else if(oss == 2){
		_delay_us(13500);
	}
	else if(oss == 3){
		_delay_us(25500);
	}
	
	long UP = read_pressure(oss);
	
	X1 = (UT-AC6)*AC5/32768;
	X2 = MC*2048/(X1+MD);
	B6 = X1+X2-4000;
	X1 = (B2*(B6*B6/4096))/2048;
	X2 = AC2 * B6/2048;
	X3 = X1 + X2;
	B3 = (((AC1*4+X3)<<oss)+2)/4;
	X1 = AC3*B6/8192;
	X2 = (B1*(B6*B6/4096))/65536;
	B4 = AC4*(unsigned long)(X3+32768)/32768;
	B7 = ((unsigned long)UP-B3) * (50000 >> oss);
	if(B7<0x80000000){p = (B7*2)/B4;}
	else{p = (B7/B4)*2;}
	X1 = (p/256)*(p/256);
	X1 = (X1 * 3038)/65536;
	X2 = (-7357*p)/65536;
	p = p + (X1 + X2 + 3791)/16; //Presion en pascales
	return p;	
	};
	
	
	
double altitude(long P){//DOnt work, i think
	double alt;
	double razon = ((double)P)/108262;
	alt =  44330*(1-pow(razon,0.1903));	
	return alt;
	};