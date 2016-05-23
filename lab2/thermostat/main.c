#include <lpc17xx.h>
#include "glcd.h"
#include "thermostat.h"

int main(void) {
    SystemInit();
    GLCD_Init();
    GLCD_Clear(White);
    GLCD_DisplayString(0, 0, 1, "Thermostat");

	init_thermostat();

    while(1) {}

    return 0;
}
