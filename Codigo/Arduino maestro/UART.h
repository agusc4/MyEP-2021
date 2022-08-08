/*
 * UART.h
 *
 */ 

#ifndef UART_H_
#define UART_H_

void mi_UART_Init( uint32_t BaudRate, uint8_t IntRX, uint8_t intTX);
int mi_putchar(char, FILE *stream);
int mi_getchar(FILE *stream);
#define fgetc() mi_getchar(&uart_io)		
#define fputc(x) mi_putchar(x,&uart_io)	

#endif /* UART_H_ */