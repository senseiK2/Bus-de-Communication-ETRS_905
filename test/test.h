#ifndef TEST_H_
#define TEST_H_

/* Tests disponibles */
#define TEST_AUCUN					0	// Pas de test : le programme principal s'execute
#define TEST_ONEWIRE_RESET			1	// Driver One-Wire : Reset et presence du capteur (test/test_onewire.c)
#define TEST_ONEWIRE_SEARCH			4	// Cahier de recette, test 1 : reconnaissance des capteurs (test/test_onewire.c)

/* Test a compiler : changer cette ligne, puis recompiler et flasher */
#define TEST_SELECTION				TEST_ONEWIRE_SEARCH

void TEST_Run(void);

#endif
