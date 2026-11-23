#include <stdint.h>
#include <stm32f446xx.h>
#include <stdio.h>
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "timer.h"
#include "util.h"
#include "test.h"


int main(void)
{
	FPU_Init();
	GPIO_Init();
	USART2_Init();
	SYSTICK_Init();
	DWT_Init();				// Necessaire avant tout appel a DWT_Delay

#if TEST_SELECTION != TEST_AUCUN
	TEST_Run();				// Test choisi dans test/test.h, ne revient pas
#endif

	while(1){
		SYSTICK_Delay(1000);
		GPIOA->ODR ^= 1<<5;
		printf("Hello\r\n");
	}
}
