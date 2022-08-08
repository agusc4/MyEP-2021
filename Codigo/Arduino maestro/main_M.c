/*
 * main_M.c
 *
 */ 

#include "main.h"

int main(void)
{	
	DDRC |=  (1<<PINC5);	// BLOQUEA MOTORES (ENABLE)
	PORTC |= (1<<PINC5);
	
	_delay_ms(400);	// PARA ESTABILIZACION DE TENSION
	mi_UART_Init(19200,1,0);
	EjesIntSPI();
	PulsInit();
	SPI_configSPIMaster();
	SPI_MasterInit();
	_delay_ms(50);
	
	flagComandoUART=0;
	
	printf("Maestro OK...\r\n");
	
	printf("Presione el pulsador \"ON\" para encender el Robot...\n\r");
	
	while(1){ if (PINC & (1<<PINC3)) {break;} }
	PORTC &=~ (1<<PINC5);
	
	printf("Encendiendo...\n\r");
	
	sei();
	
	// VELOCIDADES DE EJE EN CARGA Y VACIO PREDETERMINADAS
	v1c = v1carga;
	v1v = v1vacio;
	v2c = v2carga;
	v2v = v2vacio;
	v3c = v3carga;
	v3v = v3vacio;
	
	//PASOS DE LA TAREA DE CADA EJE
	p1 = pasos_eje_1;
	p2 = pasos_eje_2;
	p3 = pasos_eje_3;
	
	printf("Semiperiodo de ejes por defecto (en ms)\n\r");
	printf("EJE 1:\n\r -> velocidad en carga: %d \n\r -> velocidad en vacio: %d \n\r -> pasos: %d \n\r",v1c,v1v,p1);
	printf("EJE 2:\n\r -> velocidad en carga: %d \n\r -> velocidad en vacio: %d \n\r -> pasos: %d \n\r",v2c,v2v,p2);
	printf("EJE 3:\n\r -> velocidad en carga: %d \n\r -> velocidad en vacio: %d \n\r -> pasos: %d \n\r",v3c,v3v,p3);
	
	// ESPERA COMANDO DE INICIO POR UART 
	flag_param = 1;
	while (flag_param)
	{
		if (flagComandoUART) InterpretaUART();
	}
	
	// COMIENZA LA EJECUCION DE LA TAREA
	flagComandoUART = 0;
	flagPosB = 1;
	flagPosA = 0;
	flagTarea1 = 1;
	flagTarea2 = 1;
	flagTarea3 = 1;
		
	// SETEAMOS LOS VALORES ANGULARES DE CADA  EJE (VALORES FIJOS PARA LA TAREA PARTICULAR)
	/*
	ComandoEje(1,'P',57);	// config cantidad de pasos EJE 1
	ComandoEje(2,'P',65);	// config cantidad de pasos EJE 2
	ComandoEje(3,'P',16);	// config cantidad de pasos EJE 3
	*/
	
	// VALORES DE PRUEBA
	ComandoEje(1,'P',p1);	// config cantidad de pasos EJE 1
	ComandoEje(2,'P',p2);	// config cantidad de pasos EJE 2
	ComandoEje(3,'P',p3);	// config cantidad de pasos EJE 3
	
	ComandoEje(3,'E',abierto);		// estado inicial del efector final
	
	flag_param = 0;
    
	while(1){
		
		// COMPRUEBA SI HAY UN COMANDO POR LEER EN LA UART DE ENTRADA
		while (flag_param)
		{
			if (flagComandoUART) InterpretaUART();	// ESPERA COMANDO DE INICIO
		}
		// EJECUTAMOS LA TAREA
		EjecutaTarea();
		
    }	
}

/*----------------------------------------------------------------------------------------	
							ENVIO DE DATOS A LOS EJES POR SPI			
------------------------------------------------------------------------------------------*/
void ComandoEje(uint8_t eje, char parametro, uint16_t valor)
{	// LIMPIAMOS EL FLAG DE TAREA DEL EJE A COMANDAR
	
	if (eje == 1){flagTarea1 = 0;}
	if (eje == 2){flagTarea2 = 0;}
	if (eje == 3){flagTarea3 = 0;}
		
	uint8_t dummy;
	dummy = SPDR;
	
	uint8_t caract = 6;
	char str[caract];
	
	sprintf(str, ":%c%d\r",parametro,valor);
	
	switch(eje){
		case 1:
			for (int i=0; i<caract ; i++)
			{SPI_Eje1_tX(str[i]);}
				
			break;
		case 2:
			for (int i=0; i<caract ; i++)
			{SPI_Eje2_tX(str[i]);}
			break;
		case 3:
			for (int i=0; i<caract ; i++)
			{SPI_Eje3_tX(str[i]);}
			break;
		default:
			break;
	}

	// ESPERAMOS A QUE EL EJE COMPLETE LA RECEPCION DEL COMANDO
	if (eje == 1){while(flagTarea1 == 0){}}
	if (eje == 2){while(flagTarea2 == 0){}}
	if (eje == 3){while(flagTarea3 == 0){}}

}

/*----------------------------------------------------------------------------------------
*						PULSADOR ON, PARADA EMERGENCIA, SYSTEM RESET
*-----------------------------------------------------------------------------------------*/
void PulsInit(void)
{
	DDRC &=~ (1<<PINC3)|(1<<PINC4);	// PULS. ON / PULS. EMERG.

	// INTERRUPCION PARADA DE EMERGENCIA
	PCICR |= (1<<PCIE1);
	PCMSK1 |= (1<<PCINT12);	// PINC4
	PCIFR |= (1<<PCIF1);
}

/*----------------------------------------------------------------------------------------
*								INTERRUPCION PARADA DE EMERGENCIA
*-----------------------------------------------------------------------------------------*/
ISR (PCINT1_vect)
{	
	cli();
	PORTC |= (1<<PINC5);
	_delay_ms(10);
	printf("Parada de Emergencia. Reinicie el sistema\n\r");
	PCIFR |= (1<<PCIF1);
	_delay_ms(10);
	while (1){}
}

/*----------------------------------------------------------------------------------------
*								INTERRUPCIONES DE EJES
*-----------------------------------------------------------------------------------------*/
void EjesIntSPI(void)
{
	// interrupcion eje 1 y 2
	EICRA|=(1<<ISC00)|(1<<ISC01)|(1<<ISC10)|(1<<ISC11);
	EIMSK|=(1<<INT1)|(1<<INT0);
	EIFR|=(1<<INTF1)|(1<<INTF0);
	
	// interrupcion eje 3
	PCICR |= (1<<PCIE2);
	PCMSK2 |= (1<<PCINT20);
	PCIFR |= (1<<PCIF2);
	
	
	//·············································
	// PD2, PD3, PD4
	//·············································
	DDRD &=~ (1<<PIND2)|(1<<PIND3)|(1<<PIND4);	//INT. EXT. DE CADA EJE
}

/*----------------------------------------------------------------------------------------	
								INTERRUPCION EJE 1					
------------------------------------------------------------------------------------------*/
ISR (INT0_vect)
{
	flagTarea1 = 1;
	EIFR|=(1<<INTF0);
}

/*----------------------------------------------------------------------------------------	
								INTERRUPCION EJE 2				
------------------------------------------------------------------------------------------*/
ISR (INT1_vect)
{
	flagTarea2 = 1;	
	EIFR|=(1<<INTF1);
}

/*----------------------------------------------------------------------------------------	
								INTERRUPCION EJE 3				
------------------------------------------------------------------------------------------*/
ISR (PCINT2_vect)
{	
	flagTarea3 = 1;
	PCIFR|=(1<<PCIF2);
}

/*----------------------------------------------------------------------------------------	
						INTERPRETA LA TRAMA RECIBIDA POR UART (TERMINAL)					
------------------------------------------------------------------------------------------*/
void InterpretaUART(void)
{	
	int16_t valor;
	uint8_t auxEje;
	char parametro;
	
	auxEje = atoi(&comando[0]);
	parametro = comando[1];
	
	switch(auxEje){
	//Parametros Maestro	
		case 0:
			switch(comando[1])
			{
				case 'I':
				case 'i':
				printf("Comando CONTINUAR recibido por el MAESTRO\n\r");
				flag_param = 0;
				break;
				default:
				printf("Tarea interrumpida\n\r");
				flag_param = 1;
				break;
			}
		break;
	
	//Parametros Eje 1
		case 1:
			switch(comando[1])
			{
				case 'C':
				case 'c':
					valor = atoi(&comando[2]);
					v1c = valor;
					printf("Velocidad Eje 1 carga: %d\n\r",v1c); 
				break;
			
				case 'V':
				case 'v':
					valor = atoi(&comando[2]);
					v1v = valor;
					printf("Velocidad Eje 1 vacio: %d\n\r",v1v);
				break;
			
				case 'P':
				case 'p':
					valor = atoi(&comando[2]);
					p1 = valor;
					printf("Pasos Eje 1: %d\n\r",p1);
				break;
			
				default:
					printf("Comando EJE 1 incorrecto\n\r");
				break;
			
			}
		break;
	
	// Parametros eje 2
		case 2:
			switch(comando[1])
			{
				case 'C':
				case 'c':
				valor = atoi(&comando[2]);
				v2c = valor;
				printf("Velocidad Eje 2 carga: %d\n\r",v2c);
				break;
			
				case 'V':
				case 'v':
				valor = atoi(&comando[2]);
				v2v = valor;
				printf("Velocidad Eje 2 vacio: %d\n\r",v2v);
				break;
			
				case 'P':
				case 'p':
				valor = atoi(&comando[2]);
				p2 = valor;
				printf("Pasos Eje 2: %d\n\r",p2);
				break;
			
				default:
				printf("Comando EJE 2 incorrecto\n\r");
				break;
			
			}
		break;

	//Parametros eje 3
		case 3:
			switch(comando[1])
			{
				case 'C':
				case 'c':
				valor = atoi(&comando[2]);
				v3c = valor;
				printf("Velocidad Eje 3 carga: %d\n\r",v3c);
				break;
			
				case 'V':
				case 'v':
				valor = atoi(&comando[2]);
				v3v = valor;
				printf("Velocidad Eje 3 vacio: %d\n\r",v3v);
				break;
			
				case 'P':
				case 'p':
				valor = atoi(&comando[2]);
				p3 = valor;
				printf("Pasos Eje 3: %d\n\r",p3);
				break;
			
				default:
				printf("Comando EJE 3 incorrecto\n\r");
				break;
			
			}
		break;
		default:
			printf("Eje incorrecto\n\r");
		break;
	}
	flagComandoUART=0;
}

/*----------------------------------------------------------------------------------------	
								TAREA				
------------------------------------------------------------------------------------------*/
void EjecutaTarea(void)
{
	if (flagPosB && flagTarea1 && flagTarea2 && flagTarea3)
	// MOVIMIENTO DESDE LA POSICION A -> B
	{	// MOVIMIENTO EN VACIO
				
		// PASOS
		ComandoEje(1,'P',p1);	// config cantidad de pasos EJE 1
		ComandoEje(2,'P',p2);	// config cantidad de pasos EJE 2
		ComandoEje(3,'P',p3);	// config cantidad de pasos EJE 3
				
		// VELOCIDADES
		ComandoEje(1,'V',v1v);
		ComandoEje(2,'V',v2v);
		ComandoEje(3,'V',v3v);
				
		// DIRECCIONES
		ComandoEje(1,'D',der);
		ComandoEje(2,'D',der);
		ComandoEje(3,'D',der);
				
		// MOV EJE 1
		ComandoEje(1,'R',1);
		flagTarea1 = 0;
				
		// MOV EJE 2
		ComandoEje(2,'R',1);
		flagTarea2 = 0;

		while ((flagTarea1+flagTarea2)<2)	//espera a completar ambos movimientos
		{
		}
				
		// MOV EJE 3 (BAJAR)
		ComandoEje(3,'R',1);
		flagTarea3 = 0;
				
		while (flagTarea3==0)
		{
		}
		printf("ROBOT EN POSICION A\n\r");
		printf("Cerrando Efector Final...\n\r");
		ComandoEje(3,'E',cerrado);		// cierra el efector final
		ComandoEje(3,'D',izq);		// cambio de direccion de mov
		ComandoEje(3,'V',v3c);		// cambio de velocidad de mov
				
		// MOV EJE 3 (SUBIR)
		ComandoEje(3,'R',1);
				
		flagTarea3 = 0;
		while (flagTarea3==0)
		{
		}
				
		flagPosA = 1;
		flagPosB = 0;
	}
			
	if (flagPosA && flagTarea1 && flagTarea2 && flagTarea3)
	// MOVIMIENTO DESDE LA POSICION B -> A
	{	// MOVIMIENTO EN CARGA
				
		// PASOS
		ComandoEje(1,'P',p1);	// config cantidad de pasos EJE 1
		ComandoEje(2,'P',p2);	// config cantidad de pasos EJE 2
		ComandoEje(3,'P',p3);	// config cantidad de pasos EJE 3
				
		// VELOCIDADES
		ComandoEje(1,'V',v1c);
		ComandoEje(2,'V',v2c);
		ComandoEje(3,'V',v3c);
				
		// DIRECCIONES
		ComandoEje(1,'D',izq);
		ComandoEje(2,'D',izq);
		ComandoEje(3,'D',der);
				
		// MOV EJE 1
		ComandoEje(1,'R',1);
		flagTarea1 = 0;
				
		// MOV EJE 2
		ComandoEje(2,'R',1);
		flagTarea2 = 0;
				
		while ((flagTarea1+flagTarea2)<2)	//espera a completar ambos movimientos
		{
		}
				
		// MOV EJE 3 (BAJAR)
		ComandoEje(3,'R',1);
		flagTarea3 = 0;
				
		while (flagTarea3==0)
		{
		}
		printf("ROBOT EN POSICION B\n\r");
		printf("Abriendo Efector Final...\n\r");
		ComandoEje(3,'E',abierto);		// abre el efector final
		ComandoEje(3,'D',izq);		// cambio de direccion de mov
		ComandoEje(3,'V',v3v);		// cambio de velocidad de mov
				
		// MOV EJE 3 (SUBIR)
		ComandoEje(3,'R',1);
				
		flagTarea3 = 0;
		while (flagTarea3==0)
		{
		}
				
		flagPosA = 0;
		flagPosB = 1;
	}
}