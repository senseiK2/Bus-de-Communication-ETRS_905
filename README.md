# Bus de Communication ETRS_905

Mini-projet de cours du module ETRS905 « Bus de Communication » (master ESET, Université Savoie Mont Blanc).

Cible : carte Nucleo-F446RE (STM32F446RE), programmée en registres (CMSIS), sans HAL, horloge à 16 MHz (HSI).

## Organisation du dépôt

Le firmware est organisé en couches, comme la figure 1 du sujet de TP. Chaque couche n'appelle que la couche juste en dessous.

```
├── app/                 Application : main.c, puis app.c / app.h
├── lib/                 Librairies des composants : ds18b20
├── bus/                 Drivers des bus : onewire
├── low_level/
│   └── stm32/           Drivers bas niveau GPIO, TIMER, USART
├── test/                Tests de validation de chaque couche (choix du test dans test.h)
├── Drivers/CMSIS/       En-têtes CMSIS du STM32F446
├── Startup/             Fichier de démarrage généré par STM32CubeIDE
├── STM32F446RETX_FLASH.ld
└── STM32F446RETX_RAM.ld
```

| Couche | Dossier | Rôle | Exemple |
|---|---|---|---|
| Application | `app/` | Ce que fait le projet | `APP_GetTemperature()` |
| Librairie composant | `lib/` | Commandes et format de données d'un composant précis | `DS18B20_GetTemperature()` |
| Driver de bus | `bus/` | Protocole d'un bus, quel que soit le composant | `ONEWIRE_WriteByte()` |
| Bas niveau | `low_level/stm32/` | Périphériques du STM32 (broches, temporisations) | `GPIO_ResetPin()`, `DWT_Delay()` |

## Base de départ

Les fichiers de `low_level/stm32/`, `app/main.c` et `Drivers/CMSIS/` proviennent du projet « from scratch » fourni par l'enseignant :
[SylvainMontagny/STM32F446_FromScratch](https://github.com/SylvainMontagny/STM32F446_FromScratch) (commit `1a2c3e6`).

| Fichier | Rôle |
|---|---|
| `gpio.c/.h` | `GPIO_SetPin`, `GPIO_ResetPin`, `GPIO_TogglePin`, `GPIO_ReadPin`, initialisation de la LED (PA5), du bouton (PC13) et de la broche One-Wire (PA10) |
| `timer.c/.h` | Temporisation en millisecondes (`SYSTICK_Delay`) et en microsecondes (`DWT_Delay`) |
| `usart.c/.h`, `util.c` | USART2 à 115200 bauds sur PA2/PA3, redirection de `printf` vers le port série du ST-Link |
| `interrupt.c/.h` | Handler du SysTick |
| `main.h` | Constantes de mode des GPIO |
| `syscalls.c`, `sysmem.c` | Appels système de la bibliothèque C |

Modifications apportées à la base :
- `gpio.c` : `GPIO_Init()` configure PA10 (D2) en sortie open drain avec pull-up interne, pour le bus One-Wire ;
- `app/main.c` : initialise le DWT et appelle les tests (dossier `test/`), puis l'application.

## Bus One-Wire

### Câblage

| DS18B20 | Nucleo-F446RE |
|---|---|
| DQ (données) | PA10 (connecteur Arduino D2) |
| VDD | 3,3 V |
| GND | GND |

La ligne de données utilise la pull-up interne du STM32 (environ 40 kΩ). Elle suffit pour un seul capteur alimenté par VDD avec un câble court. Pour un câble long ou plusieurs capteurs, il faut une résistance externe de 4,7 kΩ vers 3,3 V (et désactiver la pull-up interne).

### Driver One-Wire (`bus/onewire.c`)

Fonctions demandées par le sujet. La broche est passée dans une structure `ONEWIRE_PINOUT` (`{port, pin}`), par exemple `{GPIOA, 10}`.

| Fonction | Séquence (durées de la figure 2 du sujet) |
|---|---|
| `ONEWIRE_Reset()` | Ligne à 0 pendant 480 µs, relâche 70 µs, lecture (0 = capteur présent), attente 410 µs. Retourne 1 si un capteur répond |
| `ONEWIRE_WriteBit0()` | Ligne à 0 pendant 60 µs, relâche 10 µs |
| `ONEWIRE_WriteBit1()` | Ligne à 0 pendant 6 µs, relâche 64 µs |
| `ONEWIRE_ReadBit()` | Ligne à 0 pendant 6 µs, relâche 9 µs, lecture du bit, attente 55 µs |
| `ONEWIRE_WriteByte()` | 8 bits, bit de poids faible en premier |
| `ONEWIRE_ReadByte()` | 8 bits, bit de poids faible en premier |

Les interruptions sont masquées pendant chaque symbole de bit (70 µs au plus), pour qu'un tick SysTick n'allonge pas un symbole de quelques microsecondes.

### Librairie DS18B20 (`lib/ds18b20.c`)

| Fonction | Rôle |
|---|---|
| `DS18B20_TestPresence()` | Retourne 1 si un capteur répond au Reset |
| `DS18B20_GetTemperature()` | Lance une conversion et retourne la température en °C (`float`) |
| `DS18B20_ReadAllMemory()` | Lit les 9 octets de la mémoire du capteur (scratchpad) |
| `DS18B20_SetAlarm()` | Enregistre les seuils d'alerte haut et bas en EEPROM |

Lecture de la température : Reset → SKIP_ROM (`0xCC`) → CONVERT_T (`0x44`) → attente → Reset → SKIP_ROM → READ_SCRATCHPAD (`0xBE`) → lecture de 2 octets.

Écarts avec la séquence du sujet, qui reprend celle du DS1820 du cours :
- **attente de 750 ms après CONVERT_T au lieu de 10 ms** : c'est le temps de conversion maximal du DS18B20 en 12 bits. Avec 10 ms, on relit l'ancienne valeur (85 °C au démarrage) ;
- **lecture de 2 octets au lieu d'un** : la température du DS18B20 est codée sur 16 bits signés, au pas de 1/16 °C.

## Tests

Le test à compiler se choisit dans `test/test.h`, en changeant la ligne `#define TEST_SELECTION`. Avec `TEST_AUCUN`, le programme principal s'exécute. Les résultats s'affichent sur le port série du ST-Link (115200 bauds, 8N1).

| Test | Ce qu'il fait | Résultat attendu |
|---|---|---|
| `TEST_ONEWIRE_RESET` | Reset toutes les secondes | « capteur present » avec le capteur, « aucun capteur » sans |
| `TEST_DS18B20_TEMPERATURE` | Lecture de la température toutes les secondes | 18 à 26 °C, qui monte quand on tient le capteur |
| `TEST_DS18B20_MEMORY` | Lecture des 9 octets de la mémoire toutes les 2 s | Configuration `7F` (12 bits), octets 5 à 7 à `FF 0C 10` |

## Ouvrir le projet

1. Cloner le dépôt.
2. Dans STM32CubeIDE : File > Import > General > Existing Projects into Workspace, puis sélectionner le dossier du dépôt.
3. Compiler (Project > Build Project), puis flasher la carte (Run).

Les chemins d'include et les dossiers compilés sont déjà réglés dans le projet.

## Test de la mise en place

Avec `TEST_AUCUN`, la LED LD2 clignote et « Hello » s'affiche chaque seconde sur le port série du ST-Link (115200 bauds, 8N1).
