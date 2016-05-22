#include <lpc17xx.h>
#include "glcd.h"
#include "morse_code.h"

int main(void) {
	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);
	GLCD_DisplayString(0, 0, 1, "Hello, world!");

	init_morse_code_fsm();
	
	return 0;
}
