#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include <stdint.h>
#include <stm32f446xx.h>

/* Broche utilisee pour le bus One-Wire */
typedef struct {
	GPIO_TypeDef*	port;		// GPIO Port : GPIOA, GPIOB...
	uint16_t		pin;		// GPIO Pin : 0 a 15
} ONEWIRE_PINOUT;

/* Commandes de recherche des composants presents sur le bus */
#define ONEWIRE_CMD_SEARCH_ROM		0xF0	// Tous les composants
#define ONEWIRE_CMD_ALARM_SEARCH	0xEC	// Seulement les composants en alarme

#define ONEWIRE_ROM_SIZE			8		// Identifiant unique : 64 bits

/* Etat d'une recherche, conserve d'un appel a l'autre de ONEWIRE_SearchNext() */
typedef struct {
	uint8_t	rom[ONEWIRE_ROM_SIZE];	// Identifiant du dernier composant trouve
	uint8_t	lastDiscrepancy;		// Position du dernier embranchement pris a 0
	uint8_t	lastDevice;				// 1 quand tous les composants ont ete trouves
} ONEWIRE_SEARCH;

uint8_t	ONEWIRE_Reset(ONEWIRE_PINOUT* oneWire_Pinout);
void	ONEWIRE_WriteBit0(ONEWIRE_PINOUT* oneWire_Pinout);
void	ONEWIRE_WriteBit1(ONEWIRE_PINOUT* oneWire_Pinout);
uint8_t	ONEWIRE_ReadBit(ONEWIRE_PINOUT* oneWire_Pinout);
void	ONEWIRE_WriteByte(ONEWIRE_PINOUT* oneWire_Pinout, uint8_t data);
uint8_t	ONEWIRE_ReadByte(ONEWIRE_PINOUT* oneWire_Pinout);

void	ONEWIRE_SearchInit(ONEWIRE_SEARCH* search);
uint8_t	ONEWIRE_SearchNext(ONEWIRE_PINOUT* oneWire_Pinout, ONEWIRE_SEARCH* search, uint8_t command);

#endif
