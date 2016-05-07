#include <lpc17xx.h>
#include "glcd.h"

int main(void) {
	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);
	GLCD_DisplayString(0, 0, 1, "Lab1!");
	return 0;
}
