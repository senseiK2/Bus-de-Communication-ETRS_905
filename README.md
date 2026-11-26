# Bus de Communication ETRS_905

Mini-projet de cours du module ETRS905 « Bus de Communication » (master ESET, Université Savoie Mont Blanc).

Cible : carte Nucleo-F446RE (STM32F446RE), programmée en registres (CMSIS), sans HAL, horloge à 16 MHz (HSI).

## Organisation du dépôt

Le firmware est organisé en couches, comme la figure 1 du sujet de TP. Chaque couche n'appelle que la couche juste en dessous.

```
├── app/                 Application : main.c, puis app.c / app.h
├── lib/                 Librairies des composants (DS18B20, SX1272, PCF8583…)
├── bus/                 Drivers des bus (One-Wire, SPI, I2C)
├── low_level/
│   └── stm32/           Drivers bas niveau GPIO, TIMER, USART
├── test/                Tests de validation de chaque couche (choix du test dans test.h)
├── Drivers/CMSIS/       En-têtes CMSIS du STM32F446
├── Startup/             Fichier de démarrage généré par STM32CubeIDE
├── STM32F446RETX_FLASH.ld
└── STM32F446RETX_RAM.ld
```

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

### Driver One-Wire (`bus/onewire.c`)

Fonctions demandées par le sujet, avec les durées de la figure 2 : `ONEWIRE_Reset()`, `ONEWIRE_WriteBit0()`, `ONEWIRE_WriteBit1()`, `ONEWIRE_ReadBit()`, `ONEWIRE_WriteByte()`, `ONEWIRE_ReadByte()`. La broche est passée dans une structure `ONEWIRE_PINOUT` (`{port, pin}`), par exemple `{GPIOA, 10}`.

**Ajout au sujet : recherche des composants du bus.** Chaque composant One-Wire a un identifiant unique de 64 bits (ROM). La recherche permet de découvrir les identifiants de tous les capteurs branchés, sans les connaître à l'avance (algorithme de la note d'application Maxim AN187).

| Fonction | Rôle |
|---|---|
| `ONEWIRE_SearchInit()` | Prépare une nouvelle recherche |
| `ONEWIRE_SearchNext()` | Trouve le composant suivant. Avec `ONEWIRE_CMD_SEARCH_ROM` (`0xF0`) : tous les composants. Avec `ONEWIRE_CMD_ALARM_SEARCH` (`0xEC`) : seulement ceux en alarme |

Utilisation :
```c
ONEWIRE_SEARCH search;
ONEWIRE_SearchInit(&search);
while (ONEWIRE_SearchNext(&oneWire_Pinout, &search, ONEWIRE_CMD_SEARCH_ROM)) {
	// search.rom contient l'identifiant du composant trouvé
}
```

### Câblage pour deux capteurs

Les capteurs se branchent en parallèle : toutes les broches de données sur PA10 (D2), tous les VDD sur 3,3 V, tous les GND sur GND. Avec deux capteurs, une résistance de pull-up externe de 4,7 kΩ entre PA10 et 3,3 V est recommandée.

## Tests

Le test à compiler se choisit dans `test/test.h`, en changeant la ligne `#define TEST_SELECTION`. Avec `TEST_AUCUN`, le programme principal s'exécute. Les résultats s'affichent sur le port série du ST-Link (115200 bauds, 8N1).

| Test | Ce qu'il fait | Résultat attendu |
|---|---|---|
| `TEST_ONEWIRE_RESET` | Reset toutes les secondes | « capteur present » avec le capteur, « aucun capteur » sans |
| `TEST_ONEWIRE_SEARCH` | Cahier de recette, test 1 : recherche des capteurs toutes les 2 s | Avec 2 capteurs : 2 identifiants différents se terminant par `28`. Avec 1 capteur : 1 seul |

## Ouvrir le projet

1. Cloner le dépôt.
2. Dans STM32CubeIDE : File > Import > General > Existing Projects into Workspace, puis sélectionner le dossier du dépôt.
3. Compiler (Project > Build Project), puis flasher la carte (Run).

Les chemins d'include et les dossiers compilés sont déjà réglés dans le projet.

## Test de la mise en place

Avec `TEST_AUCUN`, la LED LD2 clignote et « Hello » s'affiche chaque seconde sur le port série du ST-Link (115200 bauds, 8N1).
