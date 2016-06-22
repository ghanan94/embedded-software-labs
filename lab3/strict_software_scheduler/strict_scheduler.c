#include "strict_scheduler.h"
#include <lpc17xx.h>

/*
 * NAME:          MIN_TIME_BETWEEN_INTERRUPTS_MS
 *
 * DESCRIPTION:   Minimum time in milliseconds between each allowed interrupt.
 */
#define MIN_TIME_BETWEEN_INTERRUPTS_MS 10

/*
 * NAME:          TIMER0_IRQHandler
 *
 * DESCRIPTION:   Interrupt handler for TIMER0. Should fire
 *                <MIN_TIME_BETWEEN_INTERRUPTS_MS> milliseconds after a button
 *                interrupt occured so new interrupts can be allowed again.
 *                (On button interrupt, disable button interrupts for
 *                <MIN_TIME_BETWEEN_INTERRUPTS_MS> milliseconds.)
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void TIMER0_IRQHandler(void) {
    LPC_TIM0->IR |= 0x01; // Clear interrupt request
}

/*
 * NAME:          init_timer
 *
 * DESCRIPTION:   Initializes and sets up the timer to tick every
 *                <TIME_BETWEEN_BUTTON_READS_MS> ms.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_timer(void) {
    LPC_TIM0->TCR = 0x02; // Reset Timer
    LPC_TIM0->TCR = 0x00; // Complete reset, but do not enable Timer
    // Match value of 25000 means the timer's ISR will tick every 1ms
    // given below parameters.
    //(M = 100; N = 6; F = 12MHz; CCLKSEL set to divide by 4; PCLK_TIMER0 set to divice by 4
    // so 2 * M * F / (N * CCLKSEL_DIV_VALUE * PCLK_DIV_TIMER0 * 1000) = 2 * 100 * 12000000/(6 * 4 * 4 * 1000) = 25000
    // Multiplying above value by MIN_TIME_BETWEEN_INTERRUPTS_MS allows us to only
    // have to call the ISR when the minimum time between interrupts has passed
		// and we can re-enable interrupts.
    LPC_TIM0->MR0 = 25000 * MIN_TIME_BETWEEN_INTERRUPTS_MS;
    LPC_TIM0->MCR |= 0x07; // On match, generate interrupt, reset and stop counting
    NVIC_EnableIRQ(TIMER0_IRQn); // Allow for interrupts from Timer0
 }

/*
 * See strict_scheduler.h for comments.
 */
void init_strict_scheduled_button(void) {
    init_timer();
}
