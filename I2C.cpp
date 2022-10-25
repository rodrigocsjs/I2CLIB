/*
 * I2C.cpp
 *
 * Created: 05/08/2022 18:02:22
 *  Author: Rodrigo
 */ 

#include "I2C.h"

static unsigned char I2CDADDOST[0x20];
static unsigned char I2CDADOSR[0x20];
static unsigned char MASTERDADOS[0x20];
static volatile unsigned char I2CERRO;
static volatile unsigned char I2CREPSTART;
static volatile unsigned char slave;
static volatile unsigned char I2CDADOSIDT;	
static volatile unsigned char I2CSTATUS;
static volatile unsigned char I2CSTOP;
static volatile unsigned char I2CDADOSQTD;
static volatile unsigned char MASTERDADOSQTD;
static void (*SLAVERECEBE)(unsigned char*, int);
static void (*SLAVEENVIA)(void);
static volatile unsigned char I2CDADOSIDR;
static volatile unsigned char MASTERDADOSID;


//===================================================================================================
void Master_Habilitar(unsigned long frequencia)
{
	Definir_frequencia(frequencia);
	I2C_Habilitar();
}

//==================================================================================================
void Slave_Habilitar(unsigned char endereco)
{
	TWAR = (endereco << 1);
	I2C_Habilitar();
} 

//==================================================================================================
void I2C_Habilitar(void)
{
	I2CSTATUS = 0x00;   I2CSTOP = 0x01;	 I2CREPSTART = 0x00;
	PORTC |= (1 << 0x04) | (1 << 0x05);
	TWSR &= ~( (1<<TWPS0) | (1<<TWPS1) );
	TWCR = (1 << TWEN) | ( 1 << TWIE) | (1 << TWEA);
	sei();
}
//==================================================================================================

void Definir_frequencia(unsigned long frequencia)
{
	if( (frequencia < 35000) && (frequencia > 400000) )
	{
		frequencia = 100000;
	}
	TWBR = ((F_CPU / frequencia) - 16) / 2;
}

//==================================================================================================
void I2C_Desabilitar(void)
{
	TWCR &= ~((1 << TWEN) | (1 << TWIE) | (1 << TWEA));
	PORTC &= ~( (1<<4)|(1<<5) );
	
}
//==================================================================================================
void ACK(unsigned char ack)
{
	if(ack == 1)
	{
		TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWEA);
	}
	else
		{
		  TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
		}
}
//===================================================================================================
unsigned char Master_recebe(unsigned char endereco, unsigned char * bytes, unsigned char qtd, unsigned char stop)
{
    slave = ((endereco << 0x01)|0x01);
	
	if(qtd > 0x20)
	{
		return 0;
	}

	WDT_on();
	while(I2CSTATUS != 0x00)
	{
		;
	}
	WDT_off();
	
	I2CSTATUS = 0x01;	I2CSTOP = stop; I2CERRO = 0xFF;	MASTERDADOSID = 0;	MASTERDADOSQTD = (qtd - 1);  

	if(I2CREPSTART == 1)
	 {
		I2CREPSTART = 0;
		WDT_on();
		do 
		{
		  TWDR = slave;
		}while(TWCR & (1 << TWWC));
		WDT_off();
		
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
	} 
	else 
		{
			TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA) | (1 << TWINT) | (1 <<TWSTA);
		}

	WDT_on();
	while(I2CSTATUS == 0x01)
	{
		;
	}
	WDT_off();

	if (MASTERDADOSID < qtd)
	 {
		qtd = MASTERDADOSID;
	}

	for(unsigned char i = 0; i < qtd; ++i)
	{
		bytes[i] = MASTERDADOS[i];
	}

	return qtd;
}

//===================================================================================================
unsigned char Enviar_Erro(unsigned char erro)
{
	switch(erro)
	{
		case 0xFF:
				return 0;
		break;
		case 0x20:
				return 2;
		break;
		case 0x30:
			   return 3;
		break;
		default:
				return 99; 	
	}
}

//===================================================================================================
unsigned char Master_envia(unsigned char endereco, unsigned char* bytes, unsigned char qtd , unsigned char stop)
{
   slave = (endereco << 0x01);
	
	if(qtd > 0x20)
	{
		return 1;
	}

	WDT_on();
	while(I2CSTATUS != 0x00)
	{
		;
	}
	WDT_off();
	
	I2CSTATUS = 0x02;	I2CSTOP = stop; 	I2CERRO = 0xFF;	MASTERDADOSID = 0;	MASTERDADOSQTD = qtd;
	
	for(unsigned char i = 0; i < qtd; ++i)
	{
		MASTERDADOS[i] = bytes[i];
	}
		
	if(I2CREPSTART == 1) 
	{
		I2CREPSTART = 0;		
		WDT_on();
		do
		{
		  TWDR = slave;
		}
		while(TWCR & (1 << TWWC));
		WDT_off();
		
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);	
	} 
	else 
		{
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE) | (1 << TWSTA);	
		}

	
	WDT_on();
	while(I2CSTATUS == 0x02)
	{
		;
	}
	WDT_off();
	
	return Enviar_Erro(I2CERRO);
}
//===================================================================================================
unsigned char slave_envia(const unsigned char * bytes, unsigned char qtd)
{
	if(I2CSTATUS != 0x04)
	{
		return 0x02;
	}
	if((I2CDADOSQTD + qtd) > 0x20 )
	{
		return 0x01;
	}	
	for(unsigned char i = 0; i < qtd; ++i)
	{
		I2CDADDOST[I2CDADOSQTD + i] = bytes[i];
	}
	I2CDADOSQTD = I2CDADOSQTD + qtd;
	return 0;
}
//===================================================================================================

void FuncaoSlaveRecebe( void (*function)(unsigned char*, int) )
{
	SLAVERECEBE = function;
}
//===================================================================================================
void FuncaoSlaveEnvia( void (*function)(void) )
{
	SLAVEENVIA = function;
}

//===================================================================================================

void Stop(void)
{
	TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA) | (1 << TWINT) | (1 << TWSTO);
	WDT_on();
	while(TWCR & (1 << TWSTO))
	{
		;
	}
	WDT_off();
	
	I2CSTATUS = 0x00;
}
//===================================================================================================
void WDT_on(void)
{
 	cli();
	WDTCSR |=(1<<WDCE)|(1<<WDE);
 	WDTCSR = (1<<WDP0)|(1<<WDIE);
 	WDTCSR |=(1<<WDCE);
	__asm__ __volatile__ ("wdr");
	sei();
}
//===================================================================================================

//===================================================================================================
void WDT_off(void)
{
	cli();
	__asm__ __volatile__ ("wdr");
	MCUSR &= ~(1<<WDRF);
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = 0x00;
	sei();
}
//===================================================================================================
ISR(WDT_vect)
{
	DDRB |=(1<<5);
	while(1)
	{
		PORTB ^=(1<<5);
		_delay_ms(100);
	}
}
//===================================================================================================
ISR(TWI_vect)
{
	unsigned char STATUS = TWSR & 0xF8;
	switch(STATUS)
	{
		case 0x08:     
		case 0x10: 
				 TWDR = slave;
				 ACK(1);
		break;

		case 0X18:  
		case 0X28:
				if(MASTERDADOSID < MASTERDADOSQTD)
				{
					TWDR = MASTERDADOS[MASTERDADOSID++];
					ACK(1);
				}
				else
				{
					if (I2CSTOP == 1)
					{
						Stop();
					}
					else
						{
							I2CREPSTART = 1;
							TWCR = (1 << TWINT) | (1 << TWSTA)| (1 << TWEN) ;
							I2CSTATUS = 0x00;
						}
				}
		break;
		
		case 0X20:
					I2CERRO = 0X20;
					Stop();
		break;
		
		case 0X30: 
					I2CERRO = 0X30;
					Stop();
		break;
		
		case 0X38: 
					I2CERRO = 0X38;
					TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA) | (1 <<TWINT);
					I2CSTATUS = 0x00;
		break;

		case 0X50: 
					MASTERDADOS[MASTERDADOSID++] = TWDR;
	
		
		case 0X40:
					if(MASTERDADOSID < MASTERDADOSQTD)
					{
					ACK(1);
					}
						else
							{
							ACK(0);
							}
		break;
		
		case 0X58:
					MASTERDADOS[MASTERDADOSID++] = TWDR;
					if(I2CSTOP == 1)
					{
						Stop();
					} 
						else 
							{
								I2CREPSTART = 1;	
								TWCR = (1 << TWINT) | (1 << TWSTA)| (1 << TWEN) ;
								I2CSTATUS = 0x00;
							}
		break;
		
		case 0X48: 
					Stop();
		break;

		case 0X60:   
		case 0X70: 
		case 0X68:   
		case 0X78:
					I2CSTATUS = 0x03;
					I2CDADOSIDR = 0;
					ACK(1);
		break;
		
		case 0X80:       
		case 0X90: 
					if(I2CDADOSIDR < 0x20)
					{
						I2CDADOSR[I2CDADOSIDR++] = TWDR;
						ACK(1);
					}
					else
						{
						 ACK(0);
						}
		break;
		
		case 0XA0:
					TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA) | (1 <<TWINT);
					I2CSTATUS = 0x00;
					if(I2CDADOSIDR < 0x20)
					{
						I2CDADOSR[I2CDADOSIDR] = '\0';
					}
					SLAVERECEBE(I2CDADOSR, I2CDADOSIDR);
					I2CDADOSIDR = 0;
		break;
		
		case 0X88:      
		case 0X98: 
				 ACK(0);
		break;
	
		
		case 0XA8:          
		case 0XB0: 
					I2CSTATUS = 0x04;
					I2CDADOSIDT = 0X00;
					I2CDADOSQTD = 0X00;
					SLAVEENVIA();
					if(0 == I2CDADOSQTD)
					{
						I2CDADOSQTD = 1;
						I2CDADDOST[0] = 0x00;
					}
		
		case 0XB8: 
					TWDR = I2CDADDOST[I2CDADOSIDT++];

					if(I2CDADOSIDT < I2CDADOSQTD)
					{
						ACK(1);
					}
					else
						{
						 ACK(0);
						}
		break;
		
		case 0XC0: 
		case 0XC8:					
					ACK(1);
					I2CSTATUS = 0x00;
		break;

		
		case 0XF8:  
		
		break;
		
		case 0X00: 
				I2CERRO = 0X00;
				Stop();
		break;
	
	}
}