/*
 * main.h
 *
 */ 

#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU	16000000

#include <stdio.h>			
#include <stdlib.h>			
#include <util/delay.h>		
#include <avr/io.h>			
#include <avr/interrupt.h>
#include <math.h>	
#include "UART.h"
#include "SPI.h"


#define SPI_COMSIZE 10
char spi_comando[SPI_COMSIZE];		
volatile	uint8_t spi_indcom;	
volatile	uint8_t spi_flag;		

volatile int pos;
volatile int pre;
volatile int cont;
volatile int ciclos;

volatile uint16_t pasos, direccion,t;
volatile int auxef;

#define comSize 20
char comando[comSize];
volatile uint8_t	flagcom;
volatile uint8_t	flagComandoUART;
volatile uint8_t	flagComandoSPI;
volatile uint8_t	indcom;

void TareaEnd(void);
void control_pap(int pasos, int direccion, float t);
void InterpretaSPI(void);
void InitPin(void);

#endif /* MAIN_H_ */