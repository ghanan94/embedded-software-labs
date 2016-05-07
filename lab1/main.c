#include <stdio.h>
#include <lpc17xx.h>
#include "glcd.h"

int main(void) {
	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);
	GLCD_DisplayString(0, 0, 1, "Lab1!");

	//doTenMinuteClockWithCustomDelay();
	//doTenMinuteClockWithTimer();

	return 0;
}

void customDelay(unsigned int time) {
	// TODO: Below code is just a sample.
	// Must figure out the actual values for
	// a and b.

	unsigned int a;
	unsigned int b;
	
	for(a = time; a != 0; --a) {
		for (b = 100; b != 0; --b) {
			
		}
	}
}

void doTenMinuteClockWithCustomDelay() {
	unsigned int minutes = 0;
	unsigned int seconds = 0;

	char str[6];

	sprintf(str, "%02u:%02u", minutes, seconds);

	GLCD_Clear(White);
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

		GLCD_Clear(White);
		GLCD_DisplayString(0, 0, 1, str);
	}

	// Enable interrupts again
	__enable_irq();
}

void doTenMinuteClockWithTimer() {
	LPC_TIM0->TCR = 0x02; // Reset Timer
	LPC_TIM0->TCR = 0x01; // Enable Timer
	LPC_TIM0->MR0 = 2048; // Match value
	LPC_TIM0->MCR |= 0x03; // On match, generate interrupt and reset
	NVIC_EnableIRQ(TIMER0_IRQn); // Allow for interrupts from timer0
}

void TIMER0_IRQHander() {
	
}
