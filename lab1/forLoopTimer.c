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

int forLoopTimer_main(void) {
	unsigned int minutes = 0;
	unsigned int seconds = 0;
	char str[6];

	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);

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

	return 0;
}
