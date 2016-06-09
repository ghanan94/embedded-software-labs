#include "debounced_buttons.h"
#include <lpc17xx.h>
#include <stdlib.h>
#include "thermostat.h"

/*
 * NAME:          TIME_BETWEEN_BUTTON_READS_MS
 *
 * DESCRIPTION:   Time in milliseconds between reading button status.
 *                ******IF THIS NUMBER IS TO BE ANY HIGHER, MAY WANT TO
 *                RECONSIDER HOW THE TIMER TICKING AND <current_time> TRACKING
 *                IS IMPLEMENTED. Right now a timer tick occurs every
 *                <TIME_BETWEEN_BUTTON_READS_MS> milliseconds, and current_time
 *                is incremented by <TIME_BETWEEN_BUTTON_READS_MS>. This creates
 *                an inacurancy of current_time of up to
 *                <TIME_BETWEEN_BUTTON_READS_MS> milliseconds.*********
 */
#define TIME_BETWEEN_BUTTON_READS_MS 5

/*
 * NAME:          NUM_BUTTON_READS
 *
 * DESCRIPTION:   Number of times button status must be constant before an
 *                event is considered valid (for debouncing).
 *                ******IF THIS VALUE MUST BE MORE THAN 8, THE VARIABLE
 *                <button_reads>'s TYPE MUST BE UPDATED AS IT IS CURRENTLY
 *                AN UNSIGNED CHAR WHICH ONLY ALLOWS 8 BITS.*******
 */
#define NUM_BUTTON_READS 5

/*
 * NAME:          BUTTON_READ_MASK
 *
 * DESCRIPTION:   Mask for button status. If <NUM_BUTTON_READS> LSB are all 1
 *                then pressed; if <NUM_BOTTON_READS> LSB are all 0 then
 *                released; else bouncing.
 */
#define BUTTON_READ_MASK ((1 << NUM_BUTTON_READS) - 1)

/*
 * NAME:          NUM_POSSIBLE_BUTTON_TRANSITIONS
 *
 * DESCRIPTION:   Total number of button transitions.
 */
#define NUM_POSSIBLE_BUTTON_TRANSITIONS 2

/*
 * NAME:          POSSIBLE_BUTTON_TRANSITIONS
 *
 * DESCRIPTION:   Array of all possible state transitions for button.
 */
struct transition POSSIBLE_BUTTON_TRANSITIONS[] = {
    { BUTTON_RELEASED_STATE, BUTTON_PRESS_EVENT, BUTTON_PRESSED_STATE },
    { BUTTON_PRESSED_STATE, BUTTON_RELEASE_EVENT, BUTTON_RELEASED_STATE }
};

/*
 * NAME:          debounced_button_fsm
 *
 * DESCRIPTION:   Finite state machine for the debounced button.
 */
struct finite_state_machine debounced_up_button_fsm;
struct finite_state_machine debounced_down_button_fsm;

/*
 * NAME:          up_button_reads, down_button_reads
 *
 * DESCRIPTION:   Array of <NUM_BUTTON_READS> most recent reads of button.
 *                MSB contains the oldest read; LSB contains newest read.
 *                (Unsigned Char because only 8 bits is needed).
 */
unsigned char up_button_reads;
unsigned char down_button_reads;

/*
 * NAME:          debounced_up_button_state_transition
 *
 * DESCRIPTION:   Sends a request to thermostat to increase temperature
 *                on button release.
 *
 * PARAMETERS:
 *  int previous_state
 *    - State before the transition.
 *  int event
 *    - Event that triggered a state transition.
 *  int current_state
 *    - State transitioned to.
 *
 * RETURNS:
 *  N/A
 */
void debounced_up_button_state_transition(int previous_state, int event, int current_state) {
    if (previous_state == current_state) {
        // Nothing happened
        return;
    }

	if (event == BUTTON_RELEASE_EVENT) {
		increase_thermostat_set_temperature();
	}
}

/*
 * NAME:          debounced_down_button_state_transition
 *
 * DESCRIPTION:   Sends a request to thermostat to decrease temperature
 *                on button release.
 *
 * PARAMETERS:
 *  int previous_state
 *    - State before the transition.
 *  int event
 *    - Event that triggered a state transition.
 *  int current_state
 *    - State transitioned to.
 *
 * RETURNS:
 *  N/A
 */
void debounced_down_button_state_transition(int previous_state, int event, int current_state) {
    if (previous_state == current_state) {
        // Nothing happened
        return;
    }

	if (event == BUTTON_RELEASE_EVENT) {
		decrease_thermostat_set_temperature();
	}
}

/*
 * NAME:          read_debounced_buttons
 *
 * DESCRIPTION:   Read buttons status and debounce. Debounce is implemented by
 *                Polling the button's status (pressed or not) every
 *                millisecond, until it is constant <NUM_BUTTON_READS> times.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void read_debounced_buttons(void) {
    unsigned char up_button_read_masked;
    unsigned char down_button_read_masked;

    // Read new button status and shift all the previous ones
    up_button_reads <<= 1;
    down_button_reads <<= 1;

    up_button_reads |= ~(LPC_GPIO1->FIOPIN >> 23) & 0x01;
    down_button_reads |= ~(LPC_GPIO1->FIOPIN >> 25) & 0x01;

    up_button_read_masked = up_button_reads & BUTTON_READ_MASK;
    down_button_read_masked = down_button_reads & BUTTON_READ_MASK;

	// Check if up button has stopped bouncing and if it is pressed/released
    if (up_button_read_masked == BUTTON_READ_MASK) {
        // Pressed
        perform_state_transition(&debounced_up_button_fsm, BUTTON_PRESS_EVENT);
    } else if (up_button_read_masked == 0) {
        // Released
        perform_state_transition(&debounced_up_button_fsm, BUTTON_RELEASE_EVENT);
    }

	// Check if down button has stopped bouncing and if it is pressed/released
    if (down_button_read_masked == BUTTON_READ_MASK) {
        // Pressed
        perform_state_transition(&debounced_down_button_fsm, BUTTON_PRESS_EVENT);
    } else if (down_button_read_masked == 0) {
        // Released
        perform_state_transition(&debounced_down_button_fsm, BUTTON_RELEASE_EVENT);
    }
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
static void init_timer(void) {
    LPC_TIM0->TCR = 0x02; // Reset Timer
    LPC_TIM0->TCR = 0x01; // Enable Timer
    // Match value of 25000 means the timer's ISR will tick every 1ms
    // given below parameters.
    //(M = 100; N = 6; F = 12MHz; CCLKSEL set to divide by 4; PCLK_TIMER0 set to divice by 4
    // so 2 * M * F / (N * CCLKSEL_DIV_VALUE * PCLK_DIV_TIMER0 * 1000) = 2 * 100 * 12000000/(6 * 4 * 4 * 1000) = 25000
    // Multiplying above value by TIME_BETWEEN_BUTTON_READS_MS allows us to only
    // have to call the ISR when a read is supposed to occur,
    // instead of every time. At worse case, this occurs every 1 ms.
    LPC_TIM0->MR0 = 25000 * TIME_BETWEEN_BUTTON_READS_MS;
    LPC_TIM0->MCR |= 0x03; // On match, generate interrupt and reset
    NVIC_EnableIRQ(TIMER0_IRQn); // Allow for interrupts from Timer0
 }

/*
 * NAME:          TIMER0_IRQHandler
 *
 * DESCRIPTION:   Interrupt handler for TIMER0. Should fire every time a button
 *                read should occur. The current time is incremented every time
 *                this happens. Downfall of this is that timer is not as
 *                accurate as can be, but we can save many unneeded calls to
 *                the ISR when TIME_BETWEEN_BUTTON_READS_MS > 1.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void TIMER0_IRQHandler(void) {
    LPC_TIM0->IR |= 0x01; // Clear interrupt request

    read_debounced_buttons();
}

/*
 * NAME:          init_debounced_button_fsm
 *
 * DESCRIPTION:   Initializes and sets up the debounced buttons finite state
 *                machine.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_debounced_buttons_fsm(void) {
    debounced_up_button_fsm.current_state = BUTTON_RELEASED_STATE;
    debounced_up_button_fsm.num_transitions = NUM_POSSIBLE_BUTTON_TRANSITIONS;
    debounced_up_button_fsm.transitions = POSSIBLE_BUTTON_TRANSITIONS;
    debounced_up_button_fsm.transition_function = &debounced_up_button_state_transition;

    debounced_down_button_fsm.current_state = BUTTON_RELEASED_STATE;
    debounced_down_button_fsm.num_transitions = NUM_POSSIBLE_BUTTON_TRANSITIONS;
    debounced_down_button_fsm.transitions = POSSIBLE_BUTTON_TRANSITIONS;
    debounced_down_button_fsm.transition_function = &debounced_down_button_state_transition;
}

/*
 * See debounced_button.h for comments.
 */
void init_debounced_buttons(void) {
    LPC_PINCON->PINSEL3 &= ~((3 << 14) | (3 << 18)); // P1.23 & P1.25 is GPIO
    LPC_GPIO1->FIODIR &= ~((1 << 23) | (1 << 25)); // P1.23 & P1.25 is input

    up_button_reads = 0;
    down_button_reads = 0;

    init_debounced_buttons_fsm();
    init_timer();
}
