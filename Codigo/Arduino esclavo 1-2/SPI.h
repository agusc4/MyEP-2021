/*
 * SPI.h
 */ 

#ifndef SPI_H_
#define SPI_H_
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
void SPI_configSPISlave(void);
void SPI_configSPIMaster(void);
void SPI_SlaveInit(void);
char SPI_Receive(void);
void SPI_MasterInit(void);
void SPI_Maestro_tX(char cData);
void SPI_Eje1_tX(char data);
void SPI_Eje2_tX(char data);
void SPI_Eje3_tX(char data);
char SPI_RecepcionM(void);
#endif /* SPI_H_ */