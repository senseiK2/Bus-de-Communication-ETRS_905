#include <stdint.h>
#include <stdio.h>
#include <stm32f446xx.h>
#include "timer.h"
#include "onewire.h"
#include "ds18b20.h"
#include "test_ds18b20.h"

/* Broche du capteur pour les tests, configuree dans GPIO_Init() */
static ONEWIRE_PINOUT oneWire_Pinout = {GPIOA, 10};		// PA10 (D2)


/* Affiche une temperature avec 2 decimales, sans printf flottant */
static void printTemperature(float temperature)
{
	int32_t centi = (int32_t)(temperature * 100.0f);
	char sign = ' ';

	if (centi < 0) {
		sign = '-';
		centi = -centi;
	}
	printf("%c%ld.%02ld C", sign, centi / 100, centi % 100);
}


/**
 * Test de la lecture de temperature.
 * Toutes les secondes environ (conversion de 750 ms + 250 ms) : lecture et affichage de la temperature.
 * Attendu : temperature ambiante (18 a 26 C), qui monte quand on tient le capteur entre les doigts.
 */
void TEST_Ds18b20Temperature(void)
{
	float temperature;

	printf("\r\n=== Test DS18B20 : temperature ===\r\n");

	while(1){
		temperature = DS18B20_GetTemperature(&oneWire_Pinout);

		if (temperature == DS18B20_ERROR_TEMPERATURE) {
			printf("Aucun capteur\r\n");
		} else {
			printf("Temperature :");
			printTemperature(temperature);
			printf("\r\n");
		}
		SYSTICK_Delay(250);
	}
}


/**
 * Test de la lecture de la memoire (scratchpad).
 * Toutes les 2 secondes : lecture des 9 octets et decodage des principaux champs.
 * Attendu : octets 5 a 7 a FF 0C 10 (valeurs fixes du DS18B20), configuration 7F (12 bits).
 */
void TEST_Ds18b20Memory(void)
{
	uint8_t data[DS18B20_SCRATCHPAD_SIZE];

	printf("\r\n=== Test DS18B20 : memoire (scratchpad) ===\r\n");

	while(1){
		SYSTICK_Delay(2000);

		if (DS18B20_ReadAllMemory(&oneWire_Pinout, data) == 0) {
			printf("Aucun capteur\r\n");
			continue;
		}

		printf("Scratchpad :");
		for (uint8_t i = 0; i < DS18B20_SCRATCHPAD_SIZE; i++)
			printf(" %02X", data[i]);
		printf("\r\n  Temperature brute : 0x%02X%02X | TH : %d | TL : %d | Config : 0x%02X\r\n",
				data[1], data[0], (int8_t)data[2], (int8_t)data[3], data[4]);
	}
}
