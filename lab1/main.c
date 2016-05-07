#include <stdio.h>
#include <lpc17xx.h>
#include "glcd.h"

int main(void) {
	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);
	GLCD_DisplayString(0, 0, 1, "Lab1!");
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


void doTenMinuteClock() {
	unsigned int minutes = 0;
	unsigned int seconds = 0;

	char str[6];

	sprintf(str, "%02u:%02u", minutes, seconds);

	GLCD_Clear(White);
	GLCD_DisplayString(0, 0, 1, str);

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
}
