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


/* ===== Recherche des composants presents sur le bus =====
 * Chaque composant a un identifiant unique de 64 bits (ROM). La recherche parcourt ces identifiants
 * bit par bit, comme un arbre binaire (note d'application Maxim AN187) :
 *  - le maitre lit le bit, puis son complement : chaque composant encore en lice repond ;
 *    la ligne etant en ET logique, on obtient :
 *      1 puis 0 : tous les composants ont ce bit a 1
 *      0 puis 1 : tous les composants ont ce bit a 0
 *      0 puis 0 : embranchement, des composants ont 0 et d'autres 1
 *      1 puis 1 : plus aucun composant ne repond
 *  - le maitre ecrit la valeur choisie : seuls les composants qui ont ce bit restent en lice.
 * A chaque appel, on reprend le chemin precedent et on prend l'autre branche au dernier
 * embranchement pris a 0, jusqu'a avoir parcouru tous les composants. */


/**
 * Prepare une nouvelle recherche.
 */
void ONEWIRE_SearchInit(ONEWIRE_SEARCH* search)
{
	for (uint8_t i = 0; i < ONEWIRE_ROM_SIZE; i++)
		search->rom[i] = 0;
	search->lastDiscrepancy = 0;
	search->lastDevice = 0;
}


/**
 * Cherche le composant suivant sur le bus.
 * @param command ONEWIRE_CMD_SEARCH_ROM pour tous les composants,
 *                ONEWIRE_CMD_ALARM_SEARCH pour les composants en alarme seulement
 * @return 1 si un composant a ete trouve (identifiant dans search->rom), 0 s'il n'y en a plus
 */
uint8_t ONEWIRE_SearchNext(ONEWIRE_PINOUT* oneWire_Pinout, ONEWIRE_SEARCH* search, uint8_t command)
{
	uint8_t lastZero = 0;
	uint8_t bitPosition, byteIndex, bitMask;
	uint8_t idBit, cmpBit, direction;

	if (search->lastDevice)
		return 0;

	if (ONEWIRE_Reset(oneWire_Pinout) == 0) {
		ONEWIRE_SearchInit(search);
		return 0;
	}
	ONEWIRE_WriteByte(oneWire_Pinout, command);

	for (bitPosition = 1; bitPosition <= 64; bitPosition++) {
		byteIndex = (bitPosition - 1) / 8;
		bitMask = 1 << ((bitPosition - 1) % 8);

		idBit = ONEWIRE_ReadBit(oneWire_Pinout);
		cmpBit = ONEWIRE_ReadBit(oneWire_Pinout);

		if (idBit && cmpBit) {								// Plus aucun composant ne repond
			ONEWIRE_SearchInit(search);
			return 0;
		}

		if (idBit != cmpBit) {								// Tous les composants ont le meme bit
			direction = idBit;
		} else {											// Embranchement
			if (bitPosition < search->lastDiscrepancy)
				direction = (search->rom[byteIndex] & bitMask) ? 1 : 0;	// Meme chemin que la fois precedente
			else
				direction = (bitPosition == search->lastDiscrepancy);	// Autre branche au dernier embranchement
			if (direction == 0)
				lastZero = bitPosition;
		}

		if (direction) {
			search->rom[byteIndex] |= bitMask;
			ONEWIRE_WriteBit1(oneWire_Pinout);
		} else {
			search->rom[byteIndex] &= ~bitMask;
			ONEWIRE_WriteBit0(oneWire_Pinout);
		}
	}

	search->lastDiscrepancy = lastZero;
	if (lastZero == 0)
		search->lastDevice = 1;									// Plus d'embranchement a explorer

	return 1;
}

