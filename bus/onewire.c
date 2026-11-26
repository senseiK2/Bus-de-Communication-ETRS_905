#include <stdint.h>
#include <stm32f446xx.h>
#include "gpio.h"
#include "timer.h"
#include "onewire.h"

/* La broche du bus est configuree dans GPIO_Init() (gpio.c) : sortie open drain,
 * resistance de pull-up externe (4,7 kOhm vers 3,3 V).
 *  - GPIO_ResetPin tire la ligne a 0 ;
 *  - GPIO_SetPin relache la ligne, que la pull-up ramene a 1 ;
 *  - GPIO_ReadPin lit l'etat reel de la ligne.
 * Les durees sont celles de la figure 2 du sujet, en microsecondes (DWT_Delay). */


/**
 * Envoie le symbole de Reset.
 * @return 1 si au moins un composant a repondu (impulsion de presence), 0 sinon
 */
uint8_t ONEWIRE_Reset(ONEWIRE_PINOUT* oneWire_Pinout)
{
	uint8_t presence;

	GPIO_ResetPin(oneWire_Pinout->port, oneWire_Pinout->pin);		// Drive bus low
	DWT_Delay(480);
	GPIO_SetPin(oneWire_Pinout->port, oneWire_Pinout->pin);		// Release bus
	DWT_Delay(70);
	presence = (GPIO_ReadPin(oneWire_Pinout->port, oneWire_Pinout->pin) == 0);	// 0 = device present
	DWT_Delay(410);

	return presence;
}


/* Les symboles de bit durent quelques microsecondes : les interruptions (SysTick)
 * sont masquees pendant le symbole pour ne pas l'allonger, puis restaurees. */

void ONEWIRE_WriteBit0(ONEWIRE_PINOUT* oneWire_Pinout)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();

	GPIO_ResetPin(oneWire_Pinout->port, oneWire_Pinout->pin);		// Drive bus low
	DWT_Delay(60);
	GPIO_SetPin(oneWire_Pinout->port, oneWire_Pinout->pin);		// Release bus
	DWT_Delay(10);

	__set_PRIMASK(primask);
}


void ONEWIRE_WriteBit1(ONEWIRE_PINOUT* oneWire_Pinout)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();

	GPIO_ResetPin(oneWire_Pinout->port, oneWire_Pinout->pin);		// Drive bus low
	DWT_Delay(6);
	GPIO_SetPin(oneWire_Pinout->port, oneWire_Pinout->pin);		// Release bus
	DWT_Delay(64);

	__set_PRIMASK(primask);
}


/**
 * Envoie un symbole de lecture.
 * @return le bit envoye par l'esclave (0 ou 1)
 */
uint8_t ONEWIRE_ReadBit(ONEWIRE_PINOUT* oneWire_Pinout)
{
	uint8_t bit;
	uint32_t primask = __get_PRIMASK();
	__disable_irq();

	GPIO_ResetPin(oneWire_Pinout->port, oneWire_Pinout->pin);		// Drive bus low
	DWT_Delay(6);
	GPIO_SetPin(oneWire_Pinout->port, oneWire_Pinout->pin);		// Release bus
	DWT_Delay(9);
	bit = GPIO_ReadPin(oneWire_Pinout->port, oneWire_Pinout->pin);	// Sample bus
	DWT_Delay(55);

	__set_PRIMASK(primask);
	return bit;
}


/**
 * Envoie un octet, bit de poids faible en premier.
 */
void ONEWIRE_WriteByte(ONEWIRE_PINOUT* oneWire_Pinout, uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++) {
		if (data & 0x01)	ONEWIRE_WriteBit1(oneWire_Pinout);
		else				ONEWIRE_WriteBit0(oneWire_Pinout);
		data >>= 1;
	}
}


/**
 * Lit un octet, bit de poids faible en premier.
 */
uint8_t ONEWIRE_ReadByte(ONEWIRE_PINOUT* oneWire_Pinout)
{
	uint8_t data = 0;

	for (uint8_t i = 0; i < 8; i++) {
		if (ONEWIRE_ReadBit(oneWire_Pinout))
			data |= 1 << i;
	}
	return data;
}

