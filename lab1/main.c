#include <stdio.h>
#include <lpc17xx.h>
#include "glcd.h"

void customDelay(unsigned int time) {
	// looping 25000 times seems to create a 
	// delay of 1 ms, so do that a times to create
	// a delay of a (in milliseconds).

	unsigned int a;
	unsigned int b;
	
	for(a = time; a != 0; --a) {
		for (b = 25000; b != 0; --b) {
			
		}
	}
}

void doTenMinuteClockWithCustomDelay() {
	unsigned int minutes = 0;
	unsigned int seconds = 0;

	char str[6];

	sprintf(str, "%02u:%02u", minutes, seconds);

	GLCD_DisplayString(0, 0, 1, str);

	// Disable all interrupts during the delay
	// cause by the nested for loops
	__disable_irq();

	while(minutes != 10) {
		customDelay(1000);

		if (++seconds == 60) {
			++minutes;
			seconds = 0;
		}

		sprintf(str, "%02u:%02u", minutes, seconds);

		GLCD_DisplayString(0, 0, 1, str);
	}

	// Enable interrupts again
	__enable_irq();
}

unsigned int minutes = 0;
unsigned int seconds = 0;

void TIMER0_IRQHandler(void)
{
	LPC_TIM0->IR |= 0x01;
	if (++seconds == 60) {
		if (++minutes == 10) {
			LPC_TIM0->TCR = 0x00; // Disable Timer
			NVIC_DisableIRQ(TIMER0_IRQn); // Disallow for interrupts from Timer0
		}								

		seconds = 0;
	}
}

void doTenMinuteClockWithTimer() {
	char str[6];
	unsigned int old_minutes = 0;
	unsigned int old_seconds = 0;

	sprintf(str, "%02u:%02u", minutes, seconds);
	GLCD_DisplayString(0, 0, 1, str);

	LPC_TIM0->TCR = 0x02; // Reset Timer
	LPC_TIM0->TCR = 0x01;  // Enable Timer
	LPC_TIM0->MR0 = 25000000; // Match value  (M = 100; N = 6; F = 4MHz; CCLKSEL set to 4 so 2 * M * F / (N * CCLKSEL_VALUE) = 33333333)
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
}

int main(void) {
	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);
	GLCD_DisplayString(0, 0, 1, "Lab1!");

	//doTenMinuteClockWithCustomDelay();
	doTenMinuteClockWithTimer();

	return 0;
}
