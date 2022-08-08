/*
 * SPI.c
 *
 */ 

#include <stdio.h>
#include "main.h"
#include "SPI.h"

#include "SPI.h"
//Los init deben ir despues que los config.
void SPI_configSPISlave(void)
{
	SPCR &=~(1<<MSTR);	//Selección entre maestro o escalvo
	SPCR &=~(1<<DORD);	//Orden del mensaje
	SPCR &=~(1<<CPOL);	//Polaridad del clock
	SPCR &=~(1<<CPHA);	//Fase del Clock
	SPCR &=~(1<<SPR1);	//Estos bits sirven para establecer la frecuencia del clock
	SPCR |=(1<<SPR0);	//Fclock/16 (Con esta configuración)
	SPSR &=~(1<<SPI2X);
	SPCR|=(1<<SPIE);	//Habilita interrupciones por SPI
	_delay_ms(10);
}

void SPI_configSPIMaster(void)
{
	SPCR |=(1<<MSTR);	//Selección entre maestro o escalvo
	SPCR &=~(1<<DORD);	//Orden del mensaje
	SPCR &=~(1<<CPOL);	//Polaridad del clock
	SPCR &=~(1<<CPHA);	//Fase del Clock
	SPCR &=~(1<<SPR1);	//Estos bits sirven para establecer la frecuencia del clock
	SPCR |=(1<<SPR0);	//Fclock/16 (Con esta configuración)
	SPSR &=~(1<<SPI2X);
	SPCR&=~(1<<SPIE);	//Deshabilita interrupciones por SPI
	_delay_ms(10);
}
void SPI_SlaveInit(void)
{
	/* Set MISO output, all others input */
	DDRB|=(1<<DDB4);	//MISO como salida
	DDRB&=~(1<<DDB3);	//PIN MOSI como entrada
	DDRB&=~(1<<DDB2);	//SS como entrada
	DDRB&=~(1<<DDB5);	//PIN SCK como entrada
	/* Enable SPI */
	SPCR |=(1<<SPE);
	_delay_ms(10);
}

void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB&=~(1<<DDB4);	//MISO como entrada
	DDRB|=(1<<DDB3);	//PIN MOSI como salida
	DDRB|=(7<<DDB0);	//Seleccionador de esclavo, puede ser cualquier pin, en caso de ser mas de uno, agregar otra salida;
	PORTB|=(7<<PORTB0);	//Lo dejamos en 1 (es en bajo la comunicación)
	DDRB|=(1<<DDB5);	//PIN SCK como salida
	/* Enable SPI */
	SPCR |=(1<<SPE);
		
	uint8_t NADA;
	NADA = SPSR;
	NADA = SPDR;
	_delay_ms(10);
}
char SPI_Recepcion(void) //Esta función recibe un char, en el esclavo, o en el maestro
{
	return SPDR;
}
char SPI_RecepcionM(void) //Esta función recibe un char, en el maestro
{	while(!(SPSR & (1<<SPIF)))
	;
	return SPDR;
}
void SPI_Maestro_tX(char cData)
{
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
	//SPDR = 0;
}
void SPI_Eje1_tX(char dato){ //Una por esclavo
	
	DDRB|=(7<<DDB0);
	PORTB&=~(1<<PORTB0);

	SPI_Maestro_tX(dato);

	PORTB|=(1<<PORTB0);
};

void SPI_Eje2_tX(char dato){ //Una por esclavo
	
	DDRB|=(7<<DDB0);
	PORTB&=~(1<<PORTB1);

	SPI_Maestro_tX(dato);

	PORTB|=(1<<PORTB1);
};

void SPI_Eje3_tX(char dato){ //Una por esclavo
	
	DDRB|=(7<<DDB0);
	PORTB&=~(1<<PORTB2);

	SPI_Maestro_tX(dato);

	PORTB|=(1<<PORTB2);
};