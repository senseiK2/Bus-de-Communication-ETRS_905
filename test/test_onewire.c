#include <stdint.h>
#include <stdio.h>
#include <stm32f446xx.h>
#include "timer.h"
#include "onewire.h"
#include "test_onewire.h"

/* Broche du capteur pour les tests One-Wire, configuree dans GPIO_Init() */
static ONEWIRE_PINOUT oneWire_Pinout = {GPIOA, 10};		// PA10 (D2)


/**
 * Test du Reset One-Wire.
 * Toutes les secondes : envoi du Reset, puis lecture de la ligne pour savoir si le capteur repond
 * (impulsion de presence : le capteur tire la ligne a 0 apres le Reset).
 * Attendu : "capteur present" avec le capteur branche, "aucun capteur" sans.
 */
void TEST_OneWireReset(void)
{
	printf("\r\n=== Test One-Wire : Reset et presence du capteur ===\r\n");

	while(1){
		SYSTICK_Delay(1000);

		if (ONEWIRE_Reset(&oneWire_Pinout))
			printf("Reset : capteur present\r\n");
		else
			printf("Reset : aucun capteur\r\n");
	}
}


/**
 * Test 1 du cahier de recette : reconnaissance des capteurs.
 * Toutes les 2 secondes : recherche de tous les composants du bus (SEARCH ROM) et affichage
 * de leur identifiant (code famille 0x28 pour un DS18B20).
 * Attendu : 2 capteurs reconnus avec deux identifiants differents, puis 1 seul si on en debranche un.
 */
void TEST_OneWireSearch(void)
{
	ONEWIRE_SEARCH search;
	uint8_t count;

	printf("\r\n=== Test 1 : reconnaissance des capteurs ===\r\n");

	while(1){
		SYSTICK_Delay(2000);

		count = 0;
		ONEWIRE_SearchInit(&search);
		while (ONEWIRE_SearchNext(&oneWire_Pinout, &search, ONEWIRE_CMD_SEARCH_ROM)) {
			count++;
			printf("  Capteur %d :", count);
			for (int8_t i = ONEWIRE_ROM_SIZE - 1; i >= 0; i--)		// Affichage octet de poids fort en premier
				printf(" %02X", search.rom[i]);
			printf("  (%s)\r\n", (search.rom[0] == 0x28) ? "DS18B20" : "type inconnu");
		}
		printf("%d capteur(s) reconnu(s)\r\n\r\n", count);
	}
}
