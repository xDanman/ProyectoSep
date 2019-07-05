/***************************************************
 * This is a USART library for the ATMega328P.
 *
 * It can provide basic USART (serial) communication for any application.
 *
 * This code is in the public domain. Feel free to do with it whatever you want.
 * 
 * 
 * 
 * FOR STUDENTS:
 * 
 * This file will be given to you in an 'empty' state. The function bodies are
 * provided, but not their content. You are supposed to add the proper code
 * and complete these functions.
 * 
 * Hint 1: RTFM! The Atmel device datasheets contain all information necessary.
 * 
 * Hint 2: We do not expect you to write the most performance or exceptionally
 * well-engineered code, but it should work. If in doubt, sacrifice speed for
 * reliability. If you are bored, rewrite the entire library in assembler and
 * squeeze the last microsecond out of it.
 * 
 ****************************************************/


// includes
#define F_CPU	16000000UL		// MCU Clock Speed - needed for baud rate value computation
#define Fclock 16000000UL

// includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>


#include "USART_implement_me.h"


unsigned int ubrr;
int D_bits;

// The initialisation function. Call it once from your main() program before
// issuing any USART commands with the functions below!
//
// Call it at any time to change the USART communication parameters.
//
// Returns zero in case of success, non-zero for errors.
uint8_t USART_Init(struct USART_configuration config)
{
	ubrr = Fclock/config.baudrate/16-1;
	UBRR0 = 8;
	/*UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;*/
	/*Enable  and transmitter */
	UCSR0B = (1<<TXEN0) | (1<<RXEN0);
	/* Set frame format: data, stop bit y paridad */
	
	D_bits |=(config.databits>8) ?(config.databits-2):(config.databits-5);
	UCSR0C |= (D_bits<<UCSZ00);
	
	UCSR0C |= ((config.stopbits-1)<<USBS0);
	
	switch(config.parity){
		case 'N': break;
		case 'O': UCSR0C = (1<<UPM01) | (1<<UPM00);
		break;
		case 'E': UCSR0C |= (1<<UPM01);
		break;
	}
	return 0;
}



// Transmits a single character
void USART_Transmit_char(uint8_t data)
{
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1<<UDRE0)))
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}



// Transmits a given string
void USART_Transmit_String(char* string)
{
	while(*string!='\0'){
		USART_Transmit_char(*string);
		string++;
	}
}

void USART_Transmit_Wifi(uint8_t* data)
{
	int counter = 0;
	while(counter!=25){
		USART_Transmit_char(*data);
		counter = counter + 1;
		data++;
	}
}


// Receives a single character.
char USART_Receive_char(void)
{
	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)));
	
	/* Get and return received data from buffer */
	return UDR0;
}



// Receives a '\n' terminated string and writes it into a supplied buffer.
// The buffer must be guaranteed to handle at least bufflen bytes.
// Returns the number of bytes written into the buffer.
uint8_t USART_Receive_String(char* buffer, uint8_t bufflen)
{
	// Your previous code might need little adaptions here. As a nice hint: A
	// string consists of many characters. If a user gives you a buffer to fill
	// for him, and you have a buffer full of already received characters...
	int tail=0;
	char data_in=0;
	while (data_in!='\n'){
		data_in=USART_Receive_char();
		/* Store new index */
		*buffer=data_in;
		buffer++;
		tail++;
		if(tail==bufflen-1){
			break;
		}
	}

	return tail;
} 
/*
void append(struct Node*** head_ref, char new_data)		//Coloca info al final
{
	struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));  //Crea nodo
	struct Node* last = **head_ref;						//Se define después
	new_node->data  = new_data;							//Coloca dato
	new_node->next = NULL;								//Nodo nuevo será el ultimo
	if (**head_ref == NULL)								//Solo si no se ha creado un nodo, cambia el head al nodo agregado
	{
		**head_ref = new_node;
		return;
	}
	while (last->next != NULL){						//Se actualiza la posición del last hasta que sea el ultimo
		last = last->next;
	}
	last->next = new_node;							//Cambia el next del ultimo nodo
	return;
}

void FreeList(struct Node*** head_Ref){
	struct Node* head_free = **head_Ref;              //Comienzo con el header.
	
	while (head_free != NULL){						// Recorro la lista.
		struct Node* temp = head_free;					// Puntero del nodo guardado.
		head_free = head_free->next;					// Cambio head.
		free(temp);										// Borro la memoria del puntero guardado.
	}
	//head_Ref = NULL ;								// Header es empty
}*/