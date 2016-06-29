#include "bursty_scheduler.h"
#include <lpc17xx.h>
#include "glcd.h"

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

/*
 * NAME:          current_time_100_ms
 *
 * DESCRIPTION:   Current time in milliseconds * 100;
 */
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
    LPC_GPIOINT->IO2IntClr |= 1 << 10; // Clear interrupt on P2.10
    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_ClearPendingIRQ(EINT3_IRQn);
}

/*
 * NAME:          TIMER1_IRQHandler
 *
 * DESCRIPTION:   Interrupt handler for TIMER1. Should fire every
 *                <TIMER_1_TICK_EVERY_MS> milliseconds to keep track of time.
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
        LPC_GPIO1->FIOCLR = 1 << 28; // Turn off led P1.28

        if (number_of_bursts != MAX_INTERRUPTS_PER_BURST) {
            GLCD_DisplayString(0, 0, 1, "Waiting 4 Interrupt ");
        }
    } else {
        --led_time_left;
    }
}

/*
 * NAME:          EINT3_IRQHandler
 *
 * DESCRIPTION:   Interrupt handler for INT0 which is within EINT3 (P2.10).
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
            GLCD_DisplayString(0, 0, 1, "Reached Burst Size  ");
        } else {
            GLCD_DisplayString(0, 0, 1, "Handling Interrupt  ");
        }

        LPC_TIM0->TCR = 0x01; // Enable timer that will enable interrupts again
                              // after some time.

        LPC_GPIO1->FIOSET = 1 << 28; // Turn on led P1.28
        led_time_left = LED_ON_TIME_TIMES_100_MS;
    }
}

/*
 * NAME:          init_timer
 *
 * DESCRIPTION:   Initializes and sets up the timer to tick after
 *                <MIN_TIME_BETWEEN_INTERRUPTS_MS> ms and another timer
 *                to tick every <TIMER_1_TICK_EVERY_MS> ms to track time.
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
    // Multiplying above value by TIMER_1_TICK_EVERY_MS allows us to only
    // have to call the ISR when the time unit we are tracking has passed.
    LPC_TIM1->MR0 = 25000 * TIMER_1_TICK_EVERY_MS;
    LPC_TIM1->MCR |= 0x03; // On match, generate interrupt and reset
    NVIC_EnableIRQ(TIMER1_IRQn); // Allow for interrupts from Timer1
 }

/*
 * NAME:          init_led
 *
 * DESCRIPTION:   Initializes and sets up the leds.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_led(void) {
    // Init all leds so they are all off initially
	// But we will only be using P1.28
    LPC_GPIO2->FIODIR |= 0x7C; // LED on PORT2.2-PORT2.6;
    LPC_GPIO1->FIODIR |= (unsigned)(0xB << 28); // LED on PORT1.28,1.29 and 1.31

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
