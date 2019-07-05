/*
 * ProyectoTest.c
 *
 * Created: 26-06-2019 14:32:13
 * Author : Daniel
 */ 

// read and write addresses; set ADD0 = GND

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "i2c/I2C_implement_me.h"
#include "16x2/LCD16x2.h"
#include "bmp180/bmp180.h"
#include "USART/USART_implement_me.h"
#include "dht/DHT.h"
#include "RTC/rtc.h"


#define Aceptar !(PINB&(1<<PORTB0))
#define Izquierda !(PINB&(1<<PORTB1))
#define Derecha !(PINB&(1<<PORTB2))

#define s0 0
#define s1 1
#define s2 2
#define s3 3
#define s4 4
#define s5 5
#define s6 6
#define s7 7
#define s8 8
#define s9 9
#define s10 10

#define TMP102_READ	 	0x91
#define TMP102_WRITE 	0x90

void Change_max(float *h, int a);
void Change_min(float *h, int a);
void setup_adc(void);
int valorAdc(void);
void init_GPIO(void);
void Timer_setup(void);
uint8_t BCDToD (uint8_t bcdByte);
double Procio(double temp, double hum);
int debouncer(int boton);
int16_t tmp102Read(void);


/*                   Globals               */

volatile bool A_Temperatura = false;
volatile bool A_Presion = false;
volatile bool A_Humedad = false;
volatile bool A_altura = false;
volatile bool A_Nivel_Agua = false;
volatile bool A_Rocio = false;
volatile bool A_Prediccion = false;
volatile bool Mostrar_LCD = false;

//Array que envia al wifi con los datos se organiza:
uint8_t *wifi_buf;
static uint8_t Wifi_Buff[25];  //Se envia temp, hum, presion, alt, lluvia, rocio, predcc
//4bytes Temp, 4bytes Hum, 4bytes Presion, 4bytes altura, 2bytes lluvia, 4bytes Rocio, 1bytes prediccion= 23 bytes


double temperature[1];
double humidity[1];

long P;	 		//Presion
int A;  		//Agua
float altitud;
char Nivel[15]; //Nivel agua
uint8_t Prediccion;


float t_ent ;
float h_ent ;
float t_max;
float t_min = 50;
float h_max;
float h_min = 100;

uint8_t BotonA;
uint8_t BotonI;
uint8_t BotonD;
int16_t tmpint;

float at1;
uint8_t at2;
uint8_t at3;
uint8_t at4;
uint8_t at5;

rtc_t Reloj;

int main(void)
{
	float Pr;	
	uint8_t state = 0;
	uint8_t nextstate = 0;
	char buffert[6];
	char bufferh[6];
	char bufferP[8];
	char buffera[15];
	char bufferR[6];
	char bufferti[10];
	char buffertm[6];
	char bufferhm[6];
	/////////////Initialization/////
	
	init_GPIO();
	i2c_begin();
	LCD_init();
	setup_adc();		
	begin_bmp180();
	DHT_Setup();
	struct USART_configuration config_57600_8N1 = {57600, 8, 'N', 1};
	USART_Init(config_57600_8N1);
	RTC_GetDateTime(&Reloj);
	////////////////////////////////
	
	sei();
	
	
	LCD_string("Bienvenido");
	LCD_Cmd(0xC0);
	LCD_string("Sistema Clima!!");
	
	_delay_ms(3000);
	
	LCD_Cmd(0x01);// Clear
	LCD_string("Aprete boton:");
	LCD_Cmd(0xC0);
	LCD_string("Aceptar");
	
	Timer_setup();
    while (1) 
    {
		//Control LCD
		switch(state){    
	       case s0 : if(BotonA)
					 {
	                      nextstate = s1;
	                      LCD_Cmd(0x01);// Clear
	                      LCD_string("Controles:");
						  _delay_ms(2000);
						  LCD_Cmd(0x01);
						  LCD_string("Izquierda: Btn 1");
						  LCD_Cmd(0xC0);
						  LCD_string("Derecha: Btn 3");
						  BotonA = 0;
						  BotonI = 0;
						  BotonD = 0;
	                  }
	                  else
					  {
	                      nextstate = s0;
	                  }
	                  break;
					  
	       case s1 :  if(BotonA)
					  {
                            nextstate = s2;
							OCR1A = 0; //Resteo Timer

                            DHT_Read(temperature,humidity);
							tmpint = tmp102Read();
                            t_ent = (float)temperature[0];
                            ftoa(t_ent,buffert,1);
							itoa(tmpint, bufferti, 10);
                            LCD_Cmd(0x01);
                            LCD_string("Tmp.Ext:");
                            LCD_string(buffert);
							LCD_print(0xDF);
							LCD_string("C");
							LCD_Cmd(0xC0);
							LCD_string("Tmp.Int:");
							LCD_string(bufferti);
							LCD_print(0xDF);
							LCD_string("C");
							OCR1A = 0xF423;
							
							A_Temperatura = true;
							BotonA = 0;
							BotonI = 0;
							BotonD = 0;
                       }
                       else{
                          nextstate = s1;
                       }
	                   break;
					   
	       case s2 :  if(BotonI)
					  {
                        nextstate = s10;
						OCR1A = 0; //Resteo Timer
						
						DHT_Read(temperature,humidity);
						ftoa(h_max,bufferh,1);
						ftoa(h_min,bufferhm,1);
						LCD_Cmd(0x01);
						LCD_string("H.Max:");
						LCD_string(bufferh);
						LCD_string("%");
						LCD_Cmd(0xC0);
						LCD_string("H.Min:");
						LCD_string(bufferhm);
						LCD_string("%");
						
	                    OCR1A = 0xF423;
	                    A_Temperatura = true;
	                    BotonA = 0;
	                    BotonI = 0;
	                    BotonD = 0;
	                  } 
	                  else if(BotonD)
					  {
	                    nextstate = s3;
						A_Temperatura= false;
						OCR1A = 0; //Resteo Timer
						DHT_Read(temperature,humidity);
						h_ent = (float)humidity[0];
						ftoa(h_ent,bufferh,1);
						LCD_Cmd(0x01);
						LCD_string("Humedad:");
						LCD_string(bufferh);
						LCD_string("%");
						OCR1A = 0xF423;
						A_Humedad = true;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  }
	                  else
					  {
	                      nextstate = s2;
						  if(Mostrar_LCD)
						  {
							t_ent = (float)temperature[0];
							ftoa(t_ent,buffert,1);
							LCD_Cmd(0x01);
							LCD_string("Temp:");
							LCD_string(buffert);
							LCD_print(0xDF);
							LCD_string("C");
							itoa(tmpint, bufferti, 10);
							LCD_Cmd(0xC0);
							LCD_string("Tmp.Int:");
							LCD_string(bufferti);
							LCD_print(0xDF);
							LCD_string("C");
							Mostrar_LCD = false;
						  }
	                  }
	                  break;
					  
	       case s3 :  if(BotonI)
					  {
                            nextstate = s2;
							A_Humedad= false;
							OCR1A = 0; //Resteo Timer
                            DHT_Read(temperature,humidity);
                            tmpint = tmp102Read();
                            t_ent = (float)temperature[0];
                            ftoa(t_ent,buffert,1);
                            itoa(tmpint, bufferti, 10);
                            LCD_Cmd(0x01);
                            LCD_string("Tmp.Ext:");
                            LCD_string(buffert);
                            LCD_print(0xDF);
                            LCD_string("C");
                            LCD_Cmd(0xC0);
                            LCD_string("Tmp.Int:");
                            LCD_string(bufferti);
                            LCD_print(0xDF);
                            LCD_string("C");
							OCR1A = 0xF423;						
							A_Temperatura = true;
							BotonA = 0;
							BotonI = 0;
							BotonD = 0;
	                  }
	                   else if(BotonD){
	                        nextstate = s4;
							A_Humedad = false;
							OCR1A = 0; //Resteo Timer
							P = getPressure( 1 );
							sprintf(bufferP,"%ld",P);
							LCD_Cmd(0x01);// Clear
							LCD_string("Presion:");
							LCD_string(bufferP);
							LCD_string("Pa");
							OCR1A = 0xF423;
								
							A_Presion = true;
							BotonA = 0;
							BotonI = 0;
							BotonD = 0;
	                    }
	                   else{
	                       nextstate = s3;
						   if(Mostrar_LCD)
						   {
							    h_ent = (float)humidity[0];
							    ftoa(h_ent,bufferh,1);
							    LCD_Cmd(0x01);
							    LCD_string("Humedad:");
							    LCD_string(bufferh);
								LCD_string("%");
								Mostrar_LCD = false;
						   }
	                   }
	                   break;
	       case s4 :  if(BotonI){
                            nextstate = s3;
                            A_Presion = false;
							OCR1A = 0; //Resteo Timer
							DHT_Read(temperature,humidity);
							h_ent = (float)humidity[0];
							ftoa(h_ent,bufferh,1);
							
							LCD_Cmd(0x01);
							LCD_string("Humedad:");
							LCD_string(bufferh);
							LCD_string("%");
							OCR1A = 0xF423;
							A_Humedad = true;
							BotonA = 0;
							BotonI = 0;
							BotonD = 0;
	                    }
	                   else if(BotonD){
	                        nextstate = s5;
	                        A_Presion = false;
							OCR1A = 0; //Resteo Timer
							P = getPressure( 1 );
							altitud = (float) altitude(P);
							ftoa(altitud,buffera,1);
							LCD_Cmd(0x01);
							LCD_string("Altitud:");
							LCD_string(buffera);
							LCD_Cmd(0xC0);
							LCD_string("snm");
							OCR1A = 0xF423;
							A_altura = true;
							BotonA = 0;
							BotonI = 0;
							BotonD = 0;
	                    }
	                    else{
	                        nextstate = s4;
							if(Mostrar_LCD)
							{
								sprintf(bufferP,"%ld",P);
								LCD_Cmd(0x01);// Clear
								LCD_string("Presion:");
								LCD_string(bufferP);
								LCD_string("Pa");
								Mostrar_LCD = false;
							}
	                    }
	                    break;
			case s5 :  if(BotonI)
					  {
                        nextstate = s4;
						A_altura = false;
						OCR1A = 0; //Resteo Timer
                        P = getPressure( 1 );
                        sprintf(bufferP,"%ld",P);
                        LCD_Cmd(0x01);// Clear
                        LCD_string("Presion:");
                        LCD_string(bufferP);
						LCD_string("Pa");
						OCR1A = 0xF423;
						A_Presion = true;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  } 
	                  else if(BotonD)
					  {
	                    nextstate = s6;
						A_altura = false;
						OCR1A = 0; //Resteo Timer
						A = valorAdc();/*char buffer[10];itoa(rain, buffer, 10);*/
						if(A < 300){strcpy( Nivel, "Fuerte");}
						else if(A < 500){strcpy( Nivel, "Media");}
						else if(A < 800){strcpy( Nivel, "Leve");}
						else{strcpy( Nivel, "Despejado");}
						
	                    LCD_Cmd(0x01);// Clear
                        LCD_string("Lluvia:"); //Gata automatic control
						LCD_string(Nivel);
						OCR1A = 0xF423;
						A_Nivel_Agua = true;;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  }
	                  else
					  {
	                      nextstate = s5;
						  if(Mostrar_LCD)
						  {
							ftoa(altitud,buffera,1);
							LCD_Cmd(0x01);
							LCD_string("Altitud:");
							LCD_string(buffera);
							LCD_Cmd(0xC0);
							LCD_string("snm");
							Mostrar_LCD = false;
						  }
	                  }
	                  break;
			case s6 :  if(BotonI)
					  {
                        nextstate = s5;
						A_Nivel_Agua = false;
						OCR1A = 0; //Resteo Timer
                        P = getPressure( 1 );
                        altitud = (float) altitude(P);
                        ftoa(altitud,buffera,1);
                        LCD_Cmd(0x01);
                        LCD_string("Altitud:");
                        LCD_string(buffera);
                        LCD_Cmd(0xC0);
                        LCD_string("snm");
						OCR1A = 0xF423;
						A_altura= true;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  } 
	                  else if(BotonD)
					  {
	                    nextstate = s7;
						A_Nivel_Agua= false;
						OCR1A = 0; //Resteo Timer
						DHT_Read(temperature,humidity);
						Pr =(float) Procio(temperature[0],humidity[0]);
						ftoa(Pr,bufferR,1);
						LCD_Cmd(0x01);
						LCD_string("PtoRocio:");
						LCD_string(bufferR);
						LCD_print(0xDF);
						LCD_string("C");
						OCR1A = 0xF423;
						A_Rocio = true;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  }
	                  else
					  {
	                      nextstate = s6;
						  if(Mostrar_LCD)
						  {
							LCD_Cmd(0x01);// Clear
							LCD_string("Lluvia:"); //Gata automatic control
							LCD_string(Nivel);
							Mostrar_LCD = false;
						  }
	                  }
	                  break;
			case s7 :  if(BotonI)
					  {
                        nextstate = s6;
						A_Rocio = false;
						OCR1A = 0; //Resteo Timer
                        A = valorAdc();/*char buffer[10];itoa(rain, buffer, 10);*/
                        if(A < 300){strcpy( Nivel, "Fuerte");}
                     	else if(A < 500){strcpy( Nivel, "Media");}
                      	else if(A < 800){strcpy( Nivel, "Leve");}
                        else{strcpy( Nivel, "Despejado");}
                        
                        LCD_Cmd(0x01);// Clear
                        LCD_string("Lluvia:"); //Gata automatic control
                        LCD_string(Nivel);
                        OCR1A = 0xF423;
						A_Nivel_Agua = true;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  } 
	                  else if(BotonD)
					  {
	                    nextstate = s8;
						A_Rocio = false;
						OCR1A = 0; //Resteo Timer
						/*
						Prediccion/////////////////////////////////////////////////7
						*/
	                    LCD_Cmd(0x01);// Clear
                        LCD_string("Prediccion"); 
						OCR1A = 0xF423;
						A_Prediccion = true;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  }
	                  else
					  {
	                      nextstate = s7;
						  if(Mostrar_LCD)
						  { 
							Pr =(float) Procio(temperature[0],humidity[0]);
							ftoa(Pr,bufferR,1);
							LCD_Cmd(0x01);
							LCD_string("PtoRocio:");
							LCD_string(bufferR);
							LCD_print(0xDF);
							LCD_string("C");
							Mostrar_LCD = false;
						  }
	                  }
	                  break;	
			case s8 :  if(BotonI)
					  {
                        nextstate = s7;
						A_Prediccion = false;
						OCR1A = 0; //Resteo Timer
						DHT_Read(temperature,humidity);
						Pr =(float) Procio(temperature[0],humidity[0]);
						ftoa(Pr,bufferR,1);
						LCD_Cmd(0x01);
						LCD_string("PtoRocio:");
						LCD_string(bufferR);
						LCD_print(0xDF);
						LCD_string("C");
						OCR1A = 0xF423;
						A_Rocio = true;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  } 
	                  else if(BotonD)
					  {
	                    nextstate = s9;
						A_Prediccion = false;
						
						OCR1A = 0; //Resteo Timer
						ftoa(t_max,buffert,1);
						ftoa(t_min,buffertm,1);
						LCD_Cmd(0x01);
						LCD_string("T.Max:");
						LCD_string(buffert);
						LCD_print(0xDF);
						LCD_string("C");
						LCD_Cmd(0xC0);
						LCD_string("T.Min:");
						LCD_string(buffertm);
						LCD_print(0xDF);
						LCD_string("C");
						OCR1A = 0xF423;
						
						A_Temperatura = true;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  }
	                  else
					  {
	                      nextstate = s8;
						  if(Mostrar_LCD)
						  {
							  
							  /*
							  Prediccion////////////////////////////////7
							  */
							LCD_Cmd(0x01);
							LCD_string("Prediccion:");
							Mostrar_LCD = false;
						  }
	                  }
	                  break;
			case s9 :  if(BotonI)
					  {
                        nextstate = s8;
					    A_Temperatura = false;
						OCR1A = 0; //Resteo Timer
						ftoa(t_max,buffert,1);
						ftoa(t_min,buffertm,1);
						LCD_Cmd(0x01);
						LCD_string("T.Max:");
						LCD_string(buffert);
						LCD_print(0xDF);
						LCD_string("C");
						LCD_Cmd(0xC0);
						LCD_string("T.Min:");
						LCD_string(buffertm);
						LCD_print(0xDF);
						LCD_string("C");
						OCR1A = 0xF423;
						A_Prediccion = true;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  } 
	                  else if(BotonD)
					  {
	                    nextstate = s10;
	                    OCR1A = 0; //Resteo Timer
						
						DHT_Read(temperature,humidity);
						ftoa(h_max,bufferh,1);
						ftoa(h_min,bufferhm,1);
						LCD_Cmd(0x01);
						LCD_string("H.Max:");
						LCD_string(bufferh);
						LCD_string("%");
						LCD_Cmd(0xC0);
						LCD_string("H.Min:");
						LCD_string(bufferhm);
						LCD_string("%");
						
	                    OCR1A = 0xF423;
	                    A_Temperatura = true;
	                    BotonA = 0;
	                    BotonI = 0;
	                    BotonD = 0;
	                  }
	                  else
					  {
	                      nextstate = s9;
						  if(Mostrar_LCD)
						  {
								ftoa(t_max,buffert,1);
								ftoa(t_min,buffertm,1);
								LCD_Cmd(0x01);
								LCD_string("T.Max:");
								LCD_string(buffert);
								LCD_print(0xDF);
								LCD_string("C");
								LCD_Cmd(0xC0);
								LCD_string("T.Min:");
								LCD_string(buffertm);
								LCD_print(0xDF);
								LCD_string("C");
								Mostrar_LCD = false;
						  }
	                  }
	                  break;
			case s10 :  if(BotonI)
					    {
                        nextstate = s9;
					    A_Temperatura = true;
						OCR1A = 0; //Resteo Timer
						ftoa(t_max,buffert,1);
						ftoa(t_min,buffertm,1);
						LCD_Cmd(0x01);
						LCD_string("T.Max:");
						LCD_string(buffert);
						LCD_print(0xDF);
						LCD_string("C");
						LCD_Cmd(0xC0);
						LCD_string("T.Min:");
						LCD_string(buffertm);
						LCD_print(0xDF);
						LCD_string("C");
						OCR1A = 0xF423;
						BotonA = 0;
						BotonI = 0;
						BotonD = 0;
	                  } 
	                  else if(BotonD)
					  {
	                    nextstate = s2;
	                    OCR1A = 0; //Resteo Timer
						
	                    DHT_Read(temperature,humidity);
	                    tmpint = tmp102Read();
	                    t_ent = (float)temperature[0];
	                    ftoa(t_ent,buffert,1);
	                    itoa(tmpint, bufferti, 10);
	                    LCD_Cmd(0x01);
	                    LCD_string("Tmp.Ext:");
	                    LCD_string(buffert);
	                    LCD_print(0xDF);
	                    LCD_string("C");
	                    LCD_Cmd(0xC0);
	                    LCD_string("Tmp.Int:");
	                    LCD_string(bufferti);
	                    LCD_print(0xDF);
	                    LCD_string("C");
						
	                    OCR1A = 0xF423;
	                    A_Temperatura = true;
	                    BotonA = 0;
	                    BotonI = 0;
	                    BotonD = 0;
	                  }
	                  else
					  {
	                      nextstate = s10;
						  if(Mostrar_LCD)
						  {
							ftoa(h_max,bufferh,1);
							ftoa(h_min,bufferhm,1);
							LCD_Cmd(0x01);
							LCD_string("H.Max:");
							LCD_string(bufferh);
							LCD_string("%");
							LCD_Cmd(0xC0);
							LCD_string("H.Min:");
							LCD_string(bufferhm);
							LCD_string("%");
							Mostrar_LCD = false;
						  }
	                  }
	                  break;
		}
	    state=nextstate;
    }
}


/*-------------------------------------------ADC----------------------------------------------------------------*/
void setup_adc()
{
	ADMUX |= (1<<REFS0) | (0<<MUX0);// AVcc referencia, ADC0
	DIDR0 |= (1<<ADC0D);					//Desactiva salida digital
	ADCSRA |= (1<<ADPS0) + (1<<ADPS1) + (1<<ADPS2) + (1<<ADEN);//ADC on,	auto trigger enable, interrupt, /128
	ADCSRA |= (1<<ADSC);										//Start conversion
}
int valorAdc()
{
	ADCSRA |= (1<<ADSC);                //Starts a new conversion
	while(ADCSRA & (1<<ADSC));          //Wait until the conversion is done
	return ADC;
}
///////////////////////////////////////////
void init_GPIO()
{
		DDRB &= ~((1<<PORTB0) + (1<<PORTB1) + (1<<PORTB2)); //Aceptar, izquierda, derecha
		PORTB |= (1<<PORTB0) | (1<<PORTB1) | (1<<PORTB2);   //PULL-UP
		PCMSK0 |= (1<<PCINT0) + (1<<PCINT1) + (1<<PCINT2);   //habilitando interrupciones
		PCICR |= (1<<PCIE0);
}


////////////////////////////////////////
uint8_t BCDToD (uint8_t bcdByte)
{
	return (((bcdByte & 0xF0) >> 4) * 10) + (bcdByte & 0x0F);
}


double Procio(double temp, double hum)
{
	double rz = pow(humidity[0]/100,0.125);
	double Pr = rz*(112+0.9*temperature[0])+(0.1*temperature[0])-110;
	return Pr;
}
//////////////////////////////////////

int debouncer(int boton)
{ 	
	if(boton == 0)
	{
		PCMSK0 &= ~(1<<PCIE0);
		_delay_ms(20);
		if(Aceptar)	{
		PCMSK0 |= (1<<PCIE0);return 1;}
		else{PCMSK0 |= (1<<PCIE0);return 0;}
	}
	else if(boton == 1)
	{
		PCMSK0 &= ~(1<<PCIE1);
		_delay_ms(20);
		if(Izquierda){PCMSK0 |= (1<<PCIE1);return 1;}
		else{PCMSK0 |= (1<<PCIE1);return 0;}
		
	}
	else if(boton == 2)
	{
		PCMSK0 &= ~(1<<PCIE2);
		_delay_ms(20);
		if(Derecha){PCMSK0 |= (1<<PCIE2);return 1;}
		else{PCMSK0 |= (1<<PCIE2);return 0;}
	}
}


ISR(PCINT0_vect)  //Falta<------------------------------------
{
	if(Aceptar)			//Discrimina que boton se apretó
	{
		if(debouncer(0)){BotonA = 1;BotonD = 0;BotonI = 0;
		}
		else{BotonA = 0;BotonD = 0;BotonI = 0;}
	}
	else if(Izquierda)
	{
		if(debouncer(1)){BotonA = 0;BotonD = 0;BotonI = 1;}
		else{BotonA = 0;BotonD = 0;BotonI = 0;}
	}
	else if(Derecha)
	{
		if(debouncer(2)){BotonA = 0;BotonD = 1;BotonI = 0;}
		else{BotonA = 0;BotonD = 0;BotonI = 0;}
	}
}
//Timer que me setea cada cuanto hago las medicioens 1s debe estar bien y enviar los mensajes al wifi cada 2s
void Timer_setup()
{
	TCCR1B|= (1<<CS12) + (1<<WGM12);
	OCR1B = 0x7A11;							//31249 //1SEG
	OCR1A = 0xF423;							//62499 //2SEG
	TIMSK1 |= (1<<OCIE1A) + (1<<OCIE1B);		//Enciende interrupts
}


ISR(TIMER1_COMPB_vect)			//Lectura de sensores//Falta<------------------------------------
{
	if(A_Temperatura | A_Humedad | A_Rocio)
	{
		DHT_Read(temperature,humidity);
		t_ent = (float)temperature[0];
		h_ent = (float)humidity[0];
		Change_max(&t_ent,0);
		Change_max(&h_ent,1);
		Change_min(&t_ent,0);
		Change_min(&h_ent,1);
		tmpint = tmp102Read();
	}
	if(A_Presion)
	{
		P = getPressure( 1 );
	}
	if(A_altura)
	{
		P = getPressure( 1 );
		altitud = (float) altitude(P);
	}
	if(A_Nivel_Agua)
	{
		A = valorAdc();/*char buffer[10];itoa(rain, buffer, 10);*/
		if(A < 300){strcpy( Nivel, "Fuerte");}
		else if(A < 500){strcpy( Nivel, "Media");}
		else if(A < 800){strcpy( Nivel, "Leve");}
		else{strcpy( Nivel, "Despejado");}
	}
/*	if(A_Prediccion)
	{
		Arbol de decision*/
	//}
	Mostrar_LCD = true;
}

ISR(TIMER1_COMPA_vect)			//Lectura de sensores//Falta<------------------------------------
{
	DHT_Read(temperature,humidity);//Temperatura y humedad
	tmpint = tmp102Read();
	P = getPressure( 1 );		   //Presion
	altitud = (float) altitude(P); //Altitud
	A = valorAdc();	       //Lluvia
	float Pr =(float) Procio(temperature[0],humidity[0]);//Punto rocio
	if(A < 300){strcpy( Nivel, "Fuerte");}
	else if(A < 500){strcpy( Nivel, "Media");}
	else if(A < 800){strcpy( Nivel, "Leve");}
	else{strcpy( Nivel, "Despejado");}
	
	t_ent = (float)temperature[0];
	h_ent = (float)humidity[0];
	
	at1 = ((float)temperature[0])*100;
	at2 = at1/1000;
	at3 = at1/100-10*at2;
	at4 = at1/10 - 100*at2 - at3*10;
	at5 = at1- 1000*at2- 100*at3-10*at4;
	USART_Transmit_char(255);
	USART_Transmit_char(at2);
	USART_Transmit_char(at3);
	USART_Transmit_char(at4);
	USART_Transmit_char(at5);
	
	at1 = ((float)humidity[0])*100;
	at2 = at1/1000;
	at3 = at1/100-10*at2;
	at4 = at1/10 - 100*at2 - at3*10;
	at5 = at1- 1000*at2- 100*at3-10*at4;
	USART_Transmit_char(at2);
	USART_Transmit_char(at3);
	USART_Transmit_char(at4);
	USART_Transmit_char(at5);	


	at2 = P/100000;
	at3 = P/10000-10*at2;
	at4 = P/1000 - 100*at2 - at3*10;
	at5 = P/100- 1000*at2- 100*at3-10*at4;
	int at6 = P/10 - 10000*at2 - 1000*at3 - 100*at4 - 10*at5;
	int at7 = P- 100000*at2 - 10000*at3 - 1000*at4 - 100*at5 - 10*at6;
	USART_Transmit_char(at2);
	USART_Transmit_char(at3);
	USART_Transmit_char(at4);
	USART_Transmit_char(at5);
	USART_Transmit_char(at6);	
	USART_Transmit_char(at7);		
	
	
	at1 = altitud*10;
	at2 = at1/1000;
	at3 = at1/100-10*at2;
	at4 = at1/10 - 100*at2 - at3*10;
	at5 = at1- 1000*at2- 100*at3-10*at4;
	USART_Transmit_char(at2);
	USART_Transmit_char(at3);
	USART_Transmit_char(at4);
	USART_Transmit_char(at5);
	
	at2 = A/1000;
	at3 = A/100 - at2*10;
	at4 = A/10-100*at2 - at3*10;
	at5 = A - 1000*at2 - at3*100 - at2*10;
	USART_Transmit_char(at2);
	USART_Transmit_char(at3);
	USART_Transmit_char(at4);
	USART_Transmit_char(at5);
	
	
	
	at1 = Pr*100;
	at2 = at1/1000;
	at3 = at1/100-10*at2;
	at4 = at1/10 - 100*at2 - at3*10;
	at5 = at1- 1000*at2- 100*at3-10*at4;
	USART_Transmit_char(at2);
	USART_Transmit_char(at3);
	USART_Transmit_char(at4);
	USART_Transmit_char(at5);
	
	USART_Transmit_char(Prediccion);
	
	
	at1 = tmpint*100;
	at2 = at1/1000;
	at3 = at1/100-10*at2;
	at4 = at1/10 - 100*at2 - at3*10;
	at5 = at1- 1000*at2- 100*at3-10*at4;
	USART_Transmit_char(at2);
	USART_Transmit_char(at3);
	USART_Transmit_char(at4);
	USART_Transmit_char(at5);	
	
	Mostrar_LCD = true;
	//USART_Transmit_Wifi(Wifi_Buff);
	//uint8_t hola = 16;
	//USART_Transmit_char(hola);
}



int16_t tmp102Read(void)
{
	
	uint8_t msb, lsb;
	int16_t temp;
	
	
	i2cSendStart();							// send start condition
	i2cWaitForComplete();
	
	i2cSendByte(TMP102_WRITE); 				// send WRITE address of TMP102
	i2cWaitForComplete();
	i2cSendByte(0x00); 						// set TMP102 pointer register to 0 (read temperature)
	i2cWaitForComplete();
	
	i2cSendStart();							// send repeated start condition
	i2cWaitForComplete();
	
	i2cSendByte(TMP102_READ);				// send READ address of TMP102
	i2cWaitForComplete();
	i2cReceiveByte(true);					// receives one byte from the bus and ACKs it//Lo recibido es el registro de lectura
	i2cWaitForComplete();
	msb = i2cGetReceivedByte(); 			// reads the MSB (it is a 12 bit value!)
	i2cWaitForComplete();
	i2cReceiveByte(false);					// receives one byte from the bus and NAKs it (last one)
	i2cWaitForComplete();
	lsb = i2cGetReceivedByte();				// reads the LSB
	i2cWaitForComplete();
	
	i2cSendStop();							// send stop condition
	TWCR = 0;								// stop everything
	
	
	// Convert the number to an 8-bit degree Celsius value
	temp = (msb<<8) | lsb;					// combine those two values into one 16 bit value
	temp >>= 4; 							// the values are left justified; fix that by shifting 4 right
	// negative numbers are represented in two's complement, but we just shifted the value and thereby potentially messed it up
	if(temp & (1<<11))						// Hence: if it is a negative number
	temp |= 0xF800;						// restore the uppermost bits
	
	// The 12 bit value has 0.0625°C precision, which is too much for what we want (and the sensor is anyways only about 0.5°C precise)
	// 0.0625 is 1/16 -> Dividing by 16 leaves 1°C precision for the output. Note that shifting >> 4 might go wrong here for negative numbers.
	temp /= 16;
	
	return(temp);
}

void Change_max(float *h, int a)
{
	if(a)
	{
		if(*h > h_max)
		{
			h_max = *h;
		}
	}
	else
	{
		if(*h > t_max)
		{
			t_max = *h;
		}		
	}
	
}
void Change_min(float *h,int a)
{
	if(a)
	{
		if(*h < h_min)
		{
			h_min = *h;
		}
	}
	else
	{
		if(*h < t_min)
		{
			t_min = *h;
		}
	}
}

