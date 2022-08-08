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
#include <string.h>
#include "UART.h"
#include "SPI.h"


#define comSize 20
char comando[comSize];
volatile uint8_t	flagcom;
volatile uint8_t flagComandoUART;
volatile uint8_t	indcom;

#define v1carga 100
#define v1vacio 50
#define v2carga 100
#define v2vacio 50
#define v3carga 100
#define v3vacio 50

uint16_t v1c, v1v;
uint16_t v2c, v2v;
uint16_t v3c, v3v;

#define pasos_eje_1 57
#define pasos_eje_2 65
#define pasos_eje_3 16
	
uint16_t p1, p2, p3;

#define izq 0
#define der 1

#define abierto 0
#define cerrado 1

// flag de tarea terminada por EJEs
volatile uint8_t	flagTarea1;
volatile uint8_t	flagTarea2;
volatile uint8_t	flagTarea3;
volatile uint8_t	flagPosA;
volatile uint8_t	flagPosB;

// flag de parametros
volatile uint8_t	flag_param;

char comando_eje[comSize];

void InterpretaUART();
void EjesIntSPI(void);
void PulsInit(void);
void ComandoEje(uint8_t eje, char parametro, uint16_t valor);
void EjecutaTarea(void);
#define LATENCIA_EJE 50
#endif /* MAIN_H_ */