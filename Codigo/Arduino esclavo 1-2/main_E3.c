
#include "main.h"

int main(void)
{						
	_delay_ms(400);	// PARA ESTABILIZACION DE TENSION
	mi_UART_Init(19200,1,0);
	
	InitPin();
	
	SPI_configSPISlave();
	SPI_SlaveInit();
	_delay_ms(50);
	
	spi_flag = 0;
	
	direccion = 0;	//Inicializa parametros
	pasos = 10;
	t = 100;		//Semiperiodo en ms
	
	printf("Eje OK...\r\n");
	
	sei();
	
	while(1){

	}
}

/*----------------------------------------------------------------------------------------	
							FUNCION DE PARADA DE EMERGENCIA		(DESHABILITADA)
------------------------------------------------------------------------------------------*/
ISR(INT0_vect)
{	
	cli();
	TCCR1A=(0<<COM1A1)|(0<<COM1A0)|(0<<COM1B1)|(0<<COM1B0)|(0<<WGM11)|(0<<WGM10);	// apagar timer 1
	TCCR1B=(0<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(0<<CS10);
	
	PORTD &=~(1<<PORTD7);	// pin de direccion nuevamente en 0
	printf("Parada de Emergencia. Reinicie el sistema\n\r");
	EIFR|=(1<<INTF0);
	_delay_ms(10);
	while (1){}
}

/*----------------------------------------------------------------------------------------
 						INTERRUPCION POR RECEPCION SPI
------------------------------------------------------------------------------------------*/
ISR(SPI_STC_vect)
{	
	char dato;
	dato = SPDR;
	switch(dato){
		case 0:		
					
			break;
		case ':':
			spi_indcom=0;			
			spi_flag=1;				
			break;
		case 13:
			if(spi_flag){
				spi_comando[spi_indcom]=0;
				spi_flag = 0;
				InterpretaSPI();					
				}
			break;
		default:
			if(spi_flag){
				if(spi_indcom<SPI_COMSIZE)
					spi_comando[spi_indcom++]=dato;
				}
			break;
		}
}

/*----------------------------------------------------------------------------------------	
						INTERPRETA LA TRAMA RECIBIDA POR SPI					
------------------------------------------------------------------------------------------*/
void InterpretaSPI(void)		
{
	char parametro;
	uint8_t aux;
	parametro=spi_comando[0];
					
		switch(parametro){
			case 'P':
			case 'p':
			pasos=atoi(&spi_comando[1]);
			printf("El valor de pasos es: %d\r\n",pasos);
			TareaEnd();
			break;
			
			case 'd':
			case 'D':
			direccion=atoi(&spi_comando[1]);
			printf("El sentido de direccion es: %d\r\n",direccion);
			TareaEnd();
			break;
			
			case 'v':
			case 'V':
			t=atoi(&spi_comando[1]);
			printf("El tiempo de semi periodo es en ms:%d\r\n",t);
			TareaEnd();
			break;
			
			case 'e':
			case 'E':
			auxef=atoi(&spi_comando[1]);
			if(auxef==0){
				PORTD&=~(1<<PORTD5);
				printf("Efector final abierto correctamente...\r\n");
			}
			else{
				PORTD|=(1<<PORTD5);
				printf("Efector final cerrado correctamente...\r\n");
			}
			TareaEnd();
			break;
			
			case 'r':
			case 'R':
			aux=atoi(&spi_comando[1]);
			printf("Ejecutando el movimiento...\r\n");
			TareaEnd();
			control_pap(pasos,direccion,t);
			break;
			
			default:
			break;
		}
};

/*----------------------------------------------------------------------------------------	
							INTERRUPCION FIN TAREA		
------------------------------------------------------------------------------------------*/
void TareaEnd(void)
{
	//pulso de interrupcion
	PORTC^=(1<<PINC0);
	_delay_us(10);
}

/*----------------------------------------------------------------------------------------	
							INICIALIZA PINES DEL EJE
------------------------------------------------------------------------------------------*/
void InitPin(void)
{

	DDRC|=(1<<PINC0);		// Salida int SPI
	PORTC&=~(1<<PINC0);
	
	DDRD |= (1<<DDD7)|(1<<DDD5);		//Salida PD7 para controlar direccion / PD5 Efector final 
	PORTD &=~ (1<<PIND7);
	
	DDRB |= (1<<DDB1);		//Salida OC1A para pulsos.
	PORTB &=~ (1<<PINB1);
	
	EICRA|=(1<<ISC00)|(1<<ISC01);
	EIMSK|=(1<<INT0);
	EIFR|=(1<<INTF0);
	
	DDRD &=~ (1<<DDD2);		// Entrada interrupcion apagado de emergencia
	
}

/*----------------------------------------------------------------------------------------	
							FUNCION DE CONTROL DE POSICION (PASOS Y DIRECCION)
------------------------------------------------------------------------------------------*/

void control_pap(int pasos, int direccion, float t){
	//Se van a usar 2 TIMERS: el TIMER 1 genera pulsos y el TIMER 0 los cuenta
	
	cont=0;
	
	//Timer1
	TCCR1A=(0<<COM1A1)|(1<<COM1A0)|(0<<COM1B1)|(0<<COM1B0)|(0<<WGM11)|(0<<WGM10); //Modo CTCOCR1A
	TCCR1B=(0<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(0<<CS10);
	
	//Timer0
	TCCR0A=(0<<COM0A1)|(0<<COM0A0)|(0<<COM0B1)|(0<<COM0B0)|(0<<WGM01)|(0<<WGM00);
	TCCR0B=(0<<WGM02)|(1<<CS02)|(1<<CS01)|(0<<CS00);  //int externa con flanco de bajada
	
	t=(float)(t/1000);	//de ms a s
	
	// Revisar que prescaler se ajusta al tiempo solicitado
	float aux= (float)(pow(2,16)/F_CPU);
	if(t<=aux)				pre=1;
	else if(t<=aux*8)		pre=8;
	else if(t<=aux*64)		pre=64;
	else if(t<=aux*256)		pre=256;
	else if(t<=aux*1024)	pre=1024;
	else printf("Error de tiempo\r\n");
	
	OCR1A=(uint16_t)((t*F_CPU/pre)-1);
	
	// Calculo de cantidad de ciclos de pulsos
	ciclos=pasos/256;
	if (pasos>256)
	{
		OCR0A=pasos-ciclos*256-1;
		TIFR0|=(1<<TOV0);
		TIMSK0=(0<<OCIE0A)|(1<<TOIE0);
	}
	else{
		OCR0A=pasos-1;
		TIFR0|=(1<<OCF0A);
		TIMSK0=(1<<OCIE0A)|(0<<TOIE0);
	}
	
	// Valor para salida de direccion
	if(direccion==0)PORTD&=~(1<<PORTD7);
	else PORTD|=(1<<PORTD7);
	
	TCNT0=0;
	TCNT1=0;
	
	// Activa timer
	if(pre==1){TCCR1B|=(0<<CS12)|(0<<CS11)|(1<<CS10);}
	
	if(pre==8){TCCR1B|=(0<<CS12)|(1<<CS11)|(0<<CS10);}
	
	if(pre==64){TCCR1B|=(0<<CS12)|(1<<CS11)|(1<<CS10);}
		
	if(pre==256){TCCR1B|=(1<<CS12)|(0<<CS11)|(0<<CS10);}
		
	if(pre==1024){TCCR1B|=(1<<CS12)|(0<<CS11)|(1<<CS10);}
	
}

/*----------------------------------------------------------------------------------------	
								INTERRUPCIONES TIMERS		
------------------------------------------------------------------------------------------*/
ISR(TIMER0_OVF_vect)
{
	if (++cont>=ciclos){
		TIFR0|=(1<<OCF0A);
		TIMSK0|=(1<<OCIE0A);
	}
}
/*----------------------------------------------------------------------------------------*/
ISR(TIMER0_COMPA_vect)		
{
	TCCR1B= (0<<CS12)|(0<<CS11)|(0<<CS10);
	PORTD&=~(1<<PORTD7);
	TareaEnd();
}