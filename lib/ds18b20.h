#ifndef DS18B20_H_
#define DS18B20_H_

#include <stdint.h>
#include "onewire.h"

/* Commandes ROM */
#define DS18B20_CMD_SKIP_ROM			0xCC

/* Commandes memoire */
#define DS18B20_CMD_CONVERT_T			0x44
#define DS18B20_CMD_WRITE_SCRATCHPAD	0x4E
#define DS18B20_CMD_READ_SCRATCHPAD		0xBE
#define DS18B20_CMD_COPY_SCRATCHPAD		0x48

/* Memoire du capteur (scratchpad) : 9 octets */
#define DS18B20_SCRATCHPAD_SIZE			9
#define DS18B20_CONFIG_12BITS			0x7F	// Resolution 12 bits (0,0625 degC)

/* Temps de conversion maximal en 12 bits (datasheet) */
#define DS18B20_CONVERSION_TIME_MS		750

/* Valeur retournee par DS18B20_GetTemperature() si aucun capteur ne repond */
#define DS18B20_ERROR_TEMPERATURE		-1000.0f

uint8_t	DS18B20_TestPresence(ONEWIRE_PINOUT* oneWire_Pinout);
float	DS18B20_GetTemperature(ONEWIRE_PINOUT* oneWire_Pinout);
uint8_t	DS18B20_ReadAllMemory(ONEWIRE_PINOUT* oneWire_Pinout, uint8_t* data);
uint8_t	DS18B20_SetAlarm(ONEWIRE_PINOUT* oneWire_Pinout, int8_t high, int8_t low);

#endif
