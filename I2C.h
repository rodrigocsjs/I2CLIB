/*
 * I2C.h
 *
 * Created: 05/08/2022 18:02:05
 *  Author: Rodrigo
 */ 


#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <avr/interrupt.h>


#ifndef  F_CPU
#define  F_CPU 16000000UL
#endif

#include <util/delay.h>

void Master_Habilitar(unsigned long frequencia);
void Slave_Habilitar(unsigned char endereco);
void Definir_frequencia(unsigned long frequencia);
void I2C_Habilitar(void);
void I2C_Desabilitar(void);
void ACK(unsigned char ack);
void Stop(void);
void WDT_off(void);
void WDT_on(void);
unsigned char Enviar_Erro(unsigned char erro);
unsigned char Master_envia(unsigned char endereco, unsigned char* bytes, unsigned char qtd , unsigned char stop);
unsigned char Master_recebe(unsigned char endereco, unsigned char* bytes, unsigned char qtd, unsigned char stop);

void FuncaoSlaveRecebe( void (*function)(unsigned char*, int) );
void FuncaoSlaveEnvia( void (*)(void) );
unsigned char slave_envia(const unsigned char * bytes, unsigned char qtd);



#endif /* I2C_H_ */