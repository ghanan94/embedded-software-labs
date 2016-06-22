#include <lpc17xx.h>
#include "glcd.h"
#include "bursty_scheduler.h"

int main(void) {
    SystemInit();
    GLCD_Init();
    GLCD_Clear(White);
    GLCD_DisplayString(0, 0, 1, "Bursty Software Scheduler");

    init_bursty_scheduled_button();

    while (1) {}

    return 0;
}
