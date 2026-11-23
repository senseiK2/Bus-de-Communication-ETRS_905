# Bus de Communication ETRS_905

Mini-projet de cours du module ETRS905 « Bus de Communication » (master ESET, Université Savoie Mont Blanc).

Cible : carte Nucleo-F446RE (STM32F446RE), programmée en registres (CMSIS), sans HAL, horloge à 16 MHz (HSI).

## Organisation du dépôt

Le firmware est organisé en couches, comme la figure 1 du sujet de TP. Chaque couche n'appelle que la couche juste en dessous.

```
├── app/                 Application : main.c, puis app.c / app.h
├── lib/                 Librairies des composants (DS18B20, SX1272, PCF8583…)
├── drivers/             Drivers des bus (One-Wire, SPI, I2C)
├── low_level/
│   └── stm32/           Drivers bas niveau GPIO, TIMER, USART (base du prof)
├── Drivers/CMSIS/       En-têtes CMSIS du STM32F446
├── Startup/             Fichier de démarrage généré par STM32CubeIDE
├── STM32F446RETX_FLASH.ld
└── STM32F446RETX_RAM.ld
```

## Base de départ

Les fichiers de `low_level/stm32/`, `app/main.c` et `Drivers/CMSIS/` proviennent sans modification du projet « from scratch » fourni par l'enseignant :
[SylvainMontagny/STM32F446_FromScratch](https://github.com/SylvainMontagny/STM32F446_FromScratch) (commit `1a2c3e6`).

| Fichier | Rôle |
|---|---|
| `gpio.c/.h` | `GPIO_SetPin`, `GPIO_ResetPin`, `GPIO_TogglePin`, `GPIO_ReadPin`, initialisation de la LED (PA5) et du bouton (PC13) |
| `timer.c/.h` | Temporisation en millisecondes (`SYSTICK_Delay`) et en microsecondes (`DWT_Delay`) |
| `usart.c/.h`, `util.c` | USART2 à 115200 bauds sur PA2/PA3, redirection de `printf` vers le port série du ST-Link |
| `interrupt.c/.h` | Handler du SysTick |
| `main.h` | Constantes de mode des GPIO |
| `syscalls.c`, `sysmem.c` | Appels système de la bibliothèque C |

`app/main.c` est le seul fichier de la base qui évoluera : il appellera l'application.

## Ouvrir le projet

1. Cloner le dépôt.
2. Dans STM32CubeIDE : File > Import > General > Existing Projects into Workspace, puis sélectionner le dossier du dépôt.
3. Compiler (Project > Build Project), puis flasher la carte (Run).

Les chemins d'include et les dossiers compilés sont déjà réglés dans le projet.

## Test de la mise en place

Avec le `main.c` de la base, la LED LD2 clignote et « Hello » s'affiche chaque seconde sur le port série du ST-Link (115200 bauds, 8N1).
