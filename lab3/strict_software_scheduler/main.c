#include <lpc17xx.h>
#include "glcd.h"
#include "strict_scheduler.h"

int main(void) {
    SystemInit();
    GLCD_Init();
    GLCD_Clear(White);
    GLCD_DisplayString(0, 0, 1, "LAB3 - Strict ");

    init_strict_scheduled_button();

    while (1) {}

    return 0;
}
