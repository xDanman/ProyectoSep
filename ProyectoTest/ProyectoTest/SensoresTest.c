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


#define Aceptar !(PINB|(1<<PORTB0))
#define Izquierda !(PINB|(1<<PORTB1))
#define Derecha !(PINB|(1<<PORTB2))

#define s0 0
#define s1 1
#define s2 2
#define s3 3
#define s4 4

void setup_adc(void);
int valorAdc(void);
int debouncer(int boton);



/*                   Globals               */

bool A_Presion;
bool A_Temperatura_Humedad;
bool A_Nivel_Agua;
bool A_Rocio;
bool A_Prediccion;
bool Mostrar_LCD;


static long Wifi_Buff[5];

long P;	 		//Presion
float T; 		//Temperatura
long H;  		//Humedad
int A;  		//Agua
char Nivel[15]; //Nivel agua
float R;		//Punto de Rocio
char Prediccion[20];

int main(void)
{
	
	long p;
	long altura;
	
	uint8 state = 0;
	uint8 nextstate = 0;
	/////////////Initialization/////
	i2c_begin();
	LCD_init();
	setup_adc();		
	begin_bmp180();
	struct USART_configuration config_57600_8N1 = {57600, 8, 'N', 1};
	USART_Init(config_57600_8N1);
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

    while (1) 
    {
		//Control LCD
		
		/*
		
		Hacer logico con delays en cada estado, para no sobrecargar pantalla(que solo cambien en las posiciones de los valores)(COn algun interrupt de tiempo, timer)
		
		*/
		switch(state){    //Ponerle un timer para que no se cambie la pantalla tan rapido, su actualizacion de 1seg
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
                            //Calculotemp();
							//CalculoHum();
							LCD_Cmd(0x01);// Clear
                            LCD_string("Temp: "); //Gata automatic control
							/*
							Logica temperatura y humedad
							
							*/
							OCR1A = 0xF423;
							A_Temperatura_Humedad = true;
							BotonA = 0;
                       }
                       else{
                          nextstate = s1;
                       }
	                   break;
	       case s2 :  if(BotonI)
					  {
                        nextstate = s4;
						A_Temperatura_Humedad = false;
						OCR1A = 0; //Resteo Timer
                        /*Logica
						
						*/
						LCD_Cmd(0x01);// Clear
                        LCD_string("Punto Rocio: "); //Gata automatic control
						LCD_Cmd(0xC0);
						LCD_string(PRED);//Es un string de una funcion, que se sea una funcion que le asigene numeros a las diferentes predicciones que luego se le asignen a los strings
						OCR1A = 0xF423;
						A_Rocio = true;
						A_Prediccion = true;
						BotonI = 0;
	                  } 
	                  else if(BotonD)
					  {
	                    nextstate = s3;
						A_Temperatura_Humedad = false;
						OCR1A = 0; //Resteo Timer
						
						/*Logica
							*/
							
	                    LCD_Cmd(0x01);// Clear
                        LCD_string("Agua "); //Gata automatic control
						LCD_Cmd(0xC0);
						LCD_string("presion");
						
						A_Presion = true;
						A_Nivel_Agua= true;
						BotonD = 0;
	                  }
	                  else
					  {
	                      nextstate = s2;
						  if(Mostrar_LCD)
						  {
							  
							LCD_Cmd(0x01);// Clear
							LCD_string("Temp: "); //Gata automatic control
							Mostrar_LCD = false;
						  }
	                  }
	                  break;
					  
	       case s3 :  if(BotonI)
					  {
                            nextstate = s2;
							A_Presion = false;
							A_Nivel_Agua= false;
							OCR1A = 0; //Resteo Timer
                            //Calculotemp();
							//CalculoHum();
							LCD_Cmd(0x01);// Clear
                            LCD_string("Temp: "); //Gata automatic control
							
							OCR1A = 0xF423;						
							A_Temperatura_Humedad = true;
							BotonI = 0;
	                  }
	                   else if(BotonD){
	                        nextstate = s4;
							A_Presion = false;
							A_Nivel_Agua= false;
							OCR1A = 0; //Resteo Timer
							/*Logica
						
							*/
							LCD_Cmd(0x01);// Clear
							LCD_string("Punto Rocio: "); //Gata automatic control
							LCD_Cmd(0xC0);
							LCD_string(PRED);//Es un string de una funcion, que se sea una funcion que le asigene numeros a las diferentes predicciones que luego se le asignen a los strings
							OCR1A = 0xF423;
								
							A_Rocio = true;
							A_Prediccion = true;
							BotonD = 0;
	                    }
	                   else{
	                       nextstate = s3;
						   if(Mostrar_LCD)
						   {
								LCD_Cmd(0x01);// Clear
								LCD_string("Agua: "); //Gata automatic control
								LCD_Cmd(0xC0);
								LCD_string("Presion");
								Mostrar_LCD = false;
						   }
	                   }
	                   break;
	       case s4 :  if(BotonI){
                            nextstate = s3;
                            A_Rocio = false;
							A_Prediccion = false;
							
							OCR1A = 0; //Resteo Timer
						
						/*Logica
							*/
							
							LCD_Cmd(0x01);// Clear
							LCD_string("Agua: "); //Gata automatic control
							LCD_Cmd(0xC0);
							LCD_string("Presion");
						
							A_Presion = true;
							A_Nivel_Agua= true;
							BotonI = 0;
	                    }
	                   else if(BotonD){
	                        nextstate = s2;
	                        A_Rocio = false;
							A_Prediccion = false;
							OCR1A = 0; //Resteo Timer
                            //Calculotemp();
							//CalculoHum();
							LCD_Cmd(0x01);// Clear
                            LCD_string("Temp: "); //Gata automatic control
							/*
							Logica temperatura y humedad
							
							*/
							OCR1A = 0xF423;
							A_Temperatura_Humedad = true;
							BotonD = 0;
	                    }
	                    else{
	                        nextstate = s4;
							if(Mostrar_LCD)
							{
								LCD_Cmd(0x01);// Clear
								LCD_string("Punto Rocio: "); //Gata automatic control
								LCD_Cmd(0xC0);
								LCD_string(PRED);//Es un string de una funcion, que se sea una funcion que le asigene numeros a las diferentes predicciones que luego se le asignen a los strings
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
	DDRB &= ~((1<<PORTB0) | (1<<PORTB1) | (1<<PORTB2)); //Aceptar, izquierda, derecha
	PORTB |= (1<<PORTB0) | (1<<PORTB1) | (1<<PORTB2);   //PULL-UP
	CMSK0 |= (1<<PCINT0) + (1<<PCINT1) + (1<<PCINT2);   //habilitando interrupciones
	PCICR |= (1<<PCIE0);
}


////////////////////////////////////////


int debouncer(int boton)
{ 	
	if(boton == 0)
	{
		_delay_ms(25);
		if(Aceptar)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if(boton == 1)
	{
		_delay_ms(25);
		if(Izquierda)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if(boton == 2)
	{
		_delay_ms(25);
		if(Derecha)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}		
}

}
ISR(PCINT0_vect)  //Falta<------------------------------------
{
	if(Aceptar)
	{
		if(debouncer(0))
		{
			BotonA = 1;
		}
		else
		{
			BotonA = 0;
		}	
	}
	else if(Izquierda)
	{
		if(debouncer(1))
		{
			BotonI = 1;
		}
		else
		{
			BotonI = 0;
		}
	}
	else if(Derecha)
	{
		if(debouncer(2))
		{
			BotonD = 1;
		}
		else
		{
			BotonD = 0;
		}
	}
}
//Timer que me setea cada cuanto hago las medicioens 1s debe estar bien y enviar los mensajes al wifi cada 2s
void Timer_setup()
{
	TCCR1B|= (1<<CS12) + (1<<WGM12);
	OCR2A = 0x7A11;							//31249
	OCR1A = 0xF423;							//62499
	TIMSK1 |= (1<<OCIE1A) + (1<<OCIE1B);		//Enciende interrupts
}


ISR(TIMER1_COMPB_vect)			//Lectura de sensores//Falta<------------------------------------
{
	if(A_Temperatura_Humedad)
	{
		//Logica temperatura
	}
	if(A_Presion)
	{
		P = getPressure( 1 );
	}
	if(A_Nivel_Agua)
	{
		A = valorAdc();
		/*char buffer[10];
		itoa(rain, buffer, 10);*/
		if(A < 300)
		{
			
			/*USART_Transmit_String(buffer);
			USART_Transmit_String(" Heavy rain\r\n");*/
			strcpy( Nivel, "Heavy Rain");
		}
		else if(A < 500)
		{
			/*USART_Transmit_String(buffer);
			USART_Transmit_String(" Moderate rain\r\n");*/
			stpcpy( Nivel, "Moderate Rain");
		}
		else if(A < 800)
		{
			/*USART_Transmit_String(buffer);
			USART_Transmit_String(" Few drops\r\n");*/
			strcpy( Nivel, "Few Drops");
		}
		else
		{
			/*USART_Transmit_String(buffer);
			USART_Transmit_String(" No rain\r\n");*/
			strcpy( Nivel, "No Rain");
		}
	}
	if(A_Rocio)
	{
		/*Ecuacion con funciones math.h*/
	}
	if(A_Prediccion)
	{
		/*Arbol de decision*/
	}
	Mostrar_LCD = true;
}

ISR(TIMER1_COMPA_vect)			//Lectura de sensores//Falta<------------------------------------
{
	T = (long)//logic temperatura;
	Wifi_Buff[0] = T;
	H = //logic;
	Wifi_Buff[1] = H;
	P = getPressure( 1 );
	Wifi_Buff[2] = P;
	A = (long) valorAdc();
	Wifi_Buff[3] = A;
	R = (long) //logic;
	Wifi_Buff[4] = R;
			if(A < 300)
		{
			
			/*USART_Transmit_String(buffer);
			USART_Transmit_String(" Heavy rain\r\n");*/
			strcpy( Nivel, "Heavy Rain");
		}
		else if(A < 500)
		{
			/*USART_Transmit_String(buffer);
			USART_Transmit_String(" Moderate rain\r\n");*/
			stpcpy( Nivel, "Moderate Rain");
		}
		else if(A < 800)
		{
			/*USART_Transmit_String(buffer);
			USART_Transmit_String(" Few drops\r\n");*/
			strcpy( Nivel, "Few Drops");
		}
		else
		{
			/*USART_Transmit_String(buffer);
			USART_Transmit_String(" No rain\r\n");*/
			strcpy( Nivel, "No Rain");
		}
	Mostrar_LCD = true;
	//Que wifi haga logica interna para cachar la prediccion
	//Logica para enviar la wea
}





