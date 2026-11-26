#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include <stdint.h>
#include <stm32f446xx.h>

/* Broche utilisee pour le bus One-Wire */
typedef struct {
	GPIO_TypeDef*	port;		// GPIO Port : GPIOA, GPIOB...
	uint16_t		pin;		// GPIO Pin : 0 a 15
} ONEWIRE_PINOUT;

uint8_t	ONEWIRE_Reset(ONEWIRE_PINOUT* oneWire_Pinout);
void	ONEWIRE_WriteBit0(ONEWIRE_PINOUT* oneWire_Pinout);
void	ONEWIRE_WriteBit1(ONEWIRE_PINOUT* oneWire_Pinout);
uint8_t	ONEWIRE_ReadBit(ONEWIRE_PINOUT* oneWire_Pinout);
void	ONEWIRE_WriteByte(ONEWIRE_PINOUT* oneWire_Pinout, uint8_t data);
uint8_t	ONEWIRE_ReadByte(ONEWIRE_PINOUT* oneWire_Pinout);

#endif
