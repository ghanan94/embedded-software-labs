#include <lpc17xx.h>
#include "debounced_button.h"

void init_debounced_button(void) {
    LPC_PINCON->PINSEL4 &= ~(3 << 20); // P2.10 is GPIO
    LPC_GPIO2->FIODIR &= ~(1 << 10); // P2.10 is input
}
