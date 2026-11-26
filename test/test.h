#ifndef TEST_H_
#define TEST_H_

/* Tests disponibles */
#define TEST_AUCUN					0	// Pas de test : le programme principal s'execute
#define TEST_ONEWIRE_RESET			1	// Driver One-Wire : Reset et presence du capteur (test/test_onewire.c)
#define TEST_DS18B20_TEMPERATURE	2	// Librairie DS18B20 : lecture de la temperature (test/test_ds18b20.c)
#define TEST_DS18B20_MEMORY			3	// Librairie DS18B20 : lecture des 9 octets de la memoire (test/test_ds18b20.c)

/* Test a compiler : changer cette ligne, puis recompiler et flasher */
#define TEST_SELECTION				TEST_DS18B20_TEMPERATURE

void TEST_Run(void);

#endif
