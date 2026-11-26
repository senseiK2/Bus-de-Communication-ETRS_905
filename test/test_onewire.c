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
