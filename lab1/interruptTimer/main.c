#include <stdio.h>
#include <lpc17xx.h>
#include "glcd.h"

unsigned int minutes = 0;
unsigned int seconds = 0;

void TIMER0_IRQHandler(void) {
	LPC_TIM0->IR |= 0x01; // Clear interrupt request

	if (++seconds == 60) {
		if (++minutes == 10) {
			LPC_TIM0->TCR = 0x00; // Disable Timer
			NVIC_DisableIRQ(TIMER0_IRQn); // Disallow for interrupts from Timer0
		}

		seconds = 0;
	}
}

int main(void) {
	char str[6];
	unsigned int old_minutes = 0;
	unsigned int old_seconds = 0;

	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);

	sprintf(str, "%02u:%02u", minutes, seconds);

	GLCD_DisplayString(0, 0, 1, str);

	LPC_TIM0->TCR = 0x02; // Reset Timer
	LPC_TIM0->TCR = 0x01;  // Enable Timer
	LPC_TIM0->MR0 = 25000000; // Match value  (M = 100; N = 6; F = 12MHz; CCLKSEL set to divide by 4; PCLK_TIMER0 set to divice by 4
							  // so 2 * M * F / (N * CCLKSEL_DIV_VALUE * PCLK_DIV_TIMER0) = 2 * 100 * 12000000/(6 * 4 * 4) = 25000000
	LPC_TIM0->MCR |= 0x03; // On match, generate interrupt and reset
	NVIC_EnableIRQ(TIMER0_IRQn); // Allow for interrupts from Timer0

	while(1) {
		if (old_minutes != minutes	|| old_seconds != seconds) {
			old_minutes = minutes;
			old_seconds = seconds;

			sprintf(str, "%02u:%02u", minutes, seconds);
			GLCD_DisplayString(0, 0, 1, str);
		} else if (old_minutes == 10) {
			break;
		}
	}

	return 0;
}
