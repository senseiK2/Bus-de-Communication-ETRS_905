#include <stdint.h>
#include <stdio.h>
#include "test.h"
#include "test_onewire.h"
#include "test_ds18b20.h"

/**
 * Lance le test choisi par TEST_SELECTION dans test.h.
 * Les initialisations communes (GPIO, USART, SysTick, DWT) sont faites dans main.c avant l'appel.
 * Un test tourne en boucle et ne revient pas.
 */
void TEST_Run(void)
{
#if TEST_SELECTION == TEST_ONEWIRE_RESET
	TEST_OneWireReset();
#elif TEST_SELECTION == TEST_DS18B20_TEMPERATURE
	TEST_Ds18b20Temperature();
#elif TEST_SELECTION == TEST_DS18B20_MEMORY
	TEST_Ds18b20Memory();
#endif

	printf("Test %d inconnu\r\n", TEST_SELECTION);
	while(1);
}
