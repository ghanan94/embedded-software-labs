#include <lpc17xx.h>
#include "glcd.h"
#include "thermostat.h"
#include "debounced_buttons.h"

int main(void) {
    SystemInit();
    GLCD_Init();
    GLCD_Clear(White);
    GLCD_DisplayString(0, 0, 1, "Thermostat");

    init_thermostat();
    init_debounced_buttons();

    while(1) {}

    return 0;
}
