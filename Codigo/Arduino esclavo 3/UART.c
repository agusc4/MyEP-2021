/*
 * UART.c
 * Biblioteca de funciones básicas de UART y manejo de interrupción por recepción de dato.
 */ 

#include "main.h"
#include "UART.h"

FILE uart_io = FDEV_SETUP_STREAM(mi_putchar, mi_getchar, _FDEV_SETUP_RW);

void mi_UART_Init( uint32_t brate, uint8_t IntRX, uint8_t IntTX)
{
	stdout = stdin = &uart_io;
	UBRR0 = F_CPU/16/brate-1;
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	if (IntRX)
	UCSR0B|= (1<<RXCIE0);
	if (IntTX)
	UCSR0B|= (1<<TXCIE0);
	flagcom=0;		
	indcom=0;
}

int mi_putchar(char c, FILE *stream) {
	while(!(UCSR0A & (1<<UDRE0)) );
	UDR0 = c;
	return 0;
}

int mi_getchar(FILE *stream) {
	while ( !(UCSR0A & (1<<RXC0)) );
	return UDR0;
}

ISR(USART_RX_vect)
{
	char Dato;
	Dato=mi_getchar(&uart_io);
	switch(Dato)
	{
		case ':':
		flagcom=1;
		indcom=0;
		break;
		case 8:		
		if(indcom>0) indcom--;
		break;
		case 13:
		if (flagcom){
			comando[indcom]=0;
			flagComandoUART=1;
		}
		flagcom=0;
		break;
		default:
		if (indcom<comSize)
		{
			comando[indcom++]=Dato;
		}
		break;
	}
}