#include <lpc17xx.h>
#include "glcd.h"
#include "strict_scheduler.h"

int main(void) {
    SystemInit();
    GLCD_Init();
    GLCD_Clear(White);
    GLCD_DisplayString(0, 0, 1, "Strict Software Scheduler");

	init_strict_scheduled_button();

	while (1) {}

    return 0;
}
