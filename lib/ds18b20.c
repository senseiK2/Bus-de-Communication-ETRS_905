#include <stdint.h>
#include "timer.h"
#include "onewire.h"
#include "ds18b20.h"

/* Une transaction avec le DS18B20 suit toujours le meme format :
 *  (1) Reset
 *  (2) Commande ROM : SKIP_ROM, un seul capteur sur le bus
 *  (3) Commande memoire
 *  (4) Donnees */


/**
 * Verifie la presence d'au moins un capteur sur le bus.
 * @return 1 si un capteur repond au Reset, 0 sinon
 */
uint8_t DS18B20_TestPresence(ONEWIRE_PINOUT* oneWire_Pinout)
{
	return ONEWIRE_Reset(oneWire_Pinout);
}


/**
 * Lance une conversion puis lit la temperature.
 * La temperature est codee sur 16 bits signes (octets 0 et 1 du scratchpad), au pas de 1/16 degC.
 * Fonction bloquante : environ 750 ms (temps de conversion en 12 bits).
 * @return la temperature en degC, ou DS18B20_ERROR_TEMPERATURE si aucun capteur ne repond
 */
float DS18B20_GetTemperature(ONEWIRE_PINOUT* oneWire_Pinout)
{
	uint8_t lsb, msb;
	int16_t raw;

	/* Conversion de la temperature */
	if (ONEWIRE_Reset(oneWire_Pinout) == 0)
		return DS18B20_ERROR_TEMPERATURE;
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CMD_SKIP_ROM);
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CMD_CONVERT_T);
	SYSTICK_Delay(DS18B20_CONVERSION_TIME_MS);

	/* Lecture des deux octets de temperature */
	if (ONEWIRE_Reset(oneWire_Pinout) == 0)
		return DS18B20_ERROR_TEMPERATURE;
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CMD_SKIP_ROM);
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CMD_READ_SCRATCHPAD);
	lsb = ONEWIRE_ReadByte(oneWire_Pinout);
	msb = ONEWIRE_ReadByte(oneWire_Pinout);
	ONEWIRE_Reset(oneWire_Pinout);								// Fin de la lecture, les 7 autres octets sont ignores

	raw = (int16_t)((msb << 8) | lsb);
	return raw / 16.0f;
}


/**
 * Lit les 9 octets du scratchpad :
 *  [0] temperature LSB  [1] temperature MSB  [2] seuil haut TH  [3] seuil bas TL
 *  [4] configuration    [5..7] reserves      [8] CRC
 * @param data tableau d'au moins DS18B20_SCRATCHPAD_SIZE octets
 * @return 1 si la lecture a eu lieu, 0 si aucun capteur ne repond
 */
uint8_t DS18B20_ReadAllMemory(ONEWIRE_PINOUT* oneWire_Pinout, uint8_t* data)
{
	if (ONEWIRE_Reset(oneWire_Pinout) == 0)
		return 0;
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CMD_SKIP_ROM);
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CMD_READ_SCRATCHPAD);
	for (uint8_t i = 0; i < DS18B20_SCRATCHPAD_SIZE; i++)
		data[i] = ONEWIRE_ReadByte(oneWire_Pinout);

	return 1;
}


/**
 * Enregistre les seuils d'alerte haut et bas (en degC entiers).
 * Les seuils sont ecrits dans le scratchpad, puis copies en EEPROM pour etre conserves hors tension.
 * La resolution est remise a 12 bits (l'octet de configuration est ecrit en meme temps).
 * @return 1 si l'ecriture a eu lieu, 0 si aucun capteur ne repond
 */
uint8_t DS18B20_SetAlarm(ONEWIRE_PINOUT* oneWire_Pinout, int8_t high, int8_t low)
{
	/* Ecriture de TH, TL et de la configuration dans le scratchpad */
	if (ONEWIRE_Reset(oneWire_Pinout) == 0)
		return 0;
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CMD_SKIP_ROM);
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CMD_WRITE_SCRATCHPAD);
	ONEWIRE_WriteByte(oneWire_Pinout, (uint8_t)high);
	ONEWIRE_WriteByte(oneWire_Pinout, (uint8_t)low);
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CONFIG_12BITS);

	/* Copie du scratchpad en EEPROM */
	if (ONEWIRE_Reset(oneWire_Pinout) == 0)
		return 0;
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CMD_SKIP_ROM);
	ONEWIRE_WriteByte(oneWire_Pinout, DS18B20_CMD_COPY_SCRATCHPAD);
	SYSTICK_Delay(10);											// Ecriture EEPROM : 10 ms max

	return 1;
}
