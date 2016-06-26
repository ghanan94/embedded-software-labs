#include "bursty_scheduler.h"
#include <lpc17xx.h>

/*
 * NAME:          MAX_INTERRUPTS_PER_BURST
 *
 * DESCRIPTION:   Maximum INT0 interrupts per burst.
 */
#define MAX_INTERRUPTS_PER_BURST 3

/*
 * NAME:          TIMER_1_TICK_EVERY_MS
 *
 * DESCRIPTION:   Timer 1 should tick only every <TIMER_1_TICK_EVERY_MS> MS
 */
#define TIMER_1_TICK_EVERY_MS 100

/*
 * NAME:          BUTTON_DEBOUNCE_TIME_100_MS
 *
 * DESCRIPTION:   Debounce button by making sure no button interrupts within
 *                <BUTTON_DEBOUNCE_TIME_100_MS> ms.
 */
#define BUTTON_DEBOUNCE_TIME_100_MS 5

/*
 * NAME:          MIN_TIME_BETWEEN_INTERRUPT_BURSTS_MS
 *
 * DESCRIPTION:   Minimum time in milliseconds between burst is reset.
 */
#define MIN_TIME_BETWEEN_INTERRUPT_BURSTS_MS 10000

/*
 * NAME:          LED_ON_TIME_TIMES_100_MS
 *
 * DESCRIPTION:   Time in 100 milliseconds led is on for.
 */
#define LED_ON_TIME_TIMES_100_MS 2

/*
 * NAME:          led_time_left
 *
 * DESCRIPTION:   led_time_left * 100ms is the ammount of time left for led to
 *                be on.
 */
unsigned char led_time_left;

unsigned int current_time_100_ms;
/*
 * NAME:          number_of_bursts
 *
 * DESCRIPTION:   Counter for number of bursts that occured in a burst session.
 */
unsigned char number_of_bursts;

/*
 * NAME:          TIMER0_IRQHandler
 *
 * DESCRIPTION:   Interrupt handler for TIMER0. Should fire
 *                <MIN_TIME_BETWEEN_INTERRUPT_BURSTS_MS> milliseconds after a
 *                button interrupt occured so a new burst can be started.
 *                (On button interrupt, disable button interrupts if maximum
 *                number of interrupts already occured this burst.)
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void TIMER0_IRQHandler(void) {
    LPC_TIM0->IR |= 0x01; // Clear interrupt request

    // Re-enable interrupts for INT0 after the minimum time has passed.
    // Clear any active interrupt requests incase there are any.
    number_of_bursts = 0;
    NVIC_EnableIRQ(EINT3_IRQn);
	NVIC_ClearPendingIRQ(EINT3_IRQn);
}

/*
 * NAME:          TIMER1_IRQHandler
 *
 * DESCRIPTION:   Interrupt handler for TIMER1. Should fire
 *                <LED_ON_TIME_MS> milliseconds after a button
 *                interrupt occured so an led can be turned off. (On button
 *                interrupt, led turns on.)
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void TIMER1_IRQHandler(void) {
    LPC_TIM1->IR |= 0x01; // Clear interrupt request

    ++current_time_100_ms;

    if (led_time_left == 0) {
        LPC_GPIO2->FIOCLR = 1 << 6; // Turn off led P2.6
    } else {
        --led_time_left;
    }
}

/*
 * NAME:          EINT3_IRQHandler
 *
 * DESCRIPTION:   Interrupt handler for INT0.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void EINT3_IRQHandler(void) {
    static unsigned int last_interrupt_time = 0;

    LPC_GPIOINT->IO2IntClr |= 1 << 10; // Clear interrupt on P2.10

    if (current_time_100_ms - last_interrupt_time > BUTTON_DEBOUNCE_TIME_100_MS) {
        last_interrupt_time = current_time_100_ms;

        if (++number_of_bursts == MAX_INTERRUPTS_PER_BURST) {
	          // Disable Interrupts for some time.
	          NVIC_DisableIRQ(EINT3_IRQn);
	      }

	      LPC_TIM0->TCR = 0x01; // Enable timer that will enable interrupts again
	                            // after some time.

	      LPC_GPIO2->FIOSET = 1 << 6; // Turn on led P2.6
	      led_time_left = LED_ON_TIME_TIMES_100_MS;
    }
}

/*
 * NAME:          init_timer
 *
 * DESCRIPTION:   Initializes and sets up the timer to tick after
 *                <MIN_TIME_BETWEEN_INTERRUPTS_MS> ms.
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
    // Multiplying above value by MIN_TIME_BETWEEN_INTERRUPT_BURSTS_MS allows us to only
    // have to call the ISR when the minimum time between bursts has passed
    // and we can re-set bursts.
    LPC_TIM0->MR0 = 25000 * MIN_TIME_BETWEEN_INTERRUPT_BURSTS_MS;
    LPC_TIM0->MCR |= 0x07; // On match, generate interrupt, reset and stop counting
    NVIC_EnableIRQ(TIMER0_IRQn); // Allow for interrupts from Timer0

		current_time_100_ms = 0;

    // Init the timer to keep track of time. SHould tick every <TIMER_1_TICK_EVERY_MS> ms.
    LPC_TIM1->TCR = 0x02; // Reset Timer
    LPC_TIM1->TCR = 0x01; // Enable Timer
    // Match value of 25000 means the timer's ISR will tick every 1ms
    // given below parameters.
    //(M = 100; N = 6; F = 12MHz; CCLKSEL set to divide by 4; PCLK_TIMER0 set to divice by 4
    // so 2 * M * F / (N * CCLKSEL_DIV_VALUE * PCLK_DIV_TIMER0 * 1000) = 2 * 100 * 12000000/(6 * 4 * 4 * 1000) = 25000
    // Multiplying above value by MIN_TIME_BETWEEN_INTERRUPTS_MS allows us to only
    // have to call the ISR when the minimum time between interrupts has passed
    // and we can re-enable interrupts.
    LPC_TIM1->MR0 = 25000 * TIMER_1_TICK_EVERY_MS;
    LPC_TIM1->MCR |= 0x03; // On match, generate interrupt and reset
    NVIC_EnableIRQ(TIMER1_IRQn); // Allow for interrupts from Timer`
 }

/*
 * NAME:          init_led
 *
 * DESCRIPTION:   Initializes and sets up a led and a timer to keep the led on
 *                for only <LED_ON_TIME_MS> milliseconds.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_led(void) {
    // Only init the right most led
    LPC_GPIO2->FIODIR |= 1 << 6; // LED on PORT2.6;

    led_time_left = 0;
}

/*
 * See bursty_scheduler.h for comments.
 */
void init_bursty_scheduled_button(void) {
    number_of_bursts = 0;

    LPC_PINCON->PINSEL4 &= ~(3 << 20); // P2.10 (INT0) is GPIO
    LPC_GPIO2->FIODIR &= ~(1 << 10);   // P2.10 (INT0) is input
    LPC_GPIOINT->IO2IntEnR |= 1 << 10; // Interrupt on rising edge of P2.10
    NVIC_EnableIRQ(EINT3_IRQn);

    init_timer();
    init_led();
}
