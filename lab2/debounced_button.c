#include "debounced_button.h"
#include <lpc17xx.h>
#include <stdlib.h>
#include "morse_code.h"

/*
 * NAME:          TIME_BETWEEN_BUTTON_READS_MS
 *
 * DESCRIPTION:   Time in milliseconds between reading button status.
 */
#define TIME_BETWEEN_BUTTON_READS_MS 5

/*
 * NAME:          DASH_DELAY_THRESHOLD_MS
 *
 * DESCRIPTION:   Minimum time in milliseconds between a button press and
 *                button release event for it to be considered a dash.
 */
#define DASH_DELAY_THRESHOLD_MS 500

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
struct finite_state_machine debounced_button_fsm;

/*
 * NAME:          current_time
 *
 * DESCRIPTION:   Current time in ms (from start of system).
 */
unsigned int current_time;

/*
 * NAME:          last_button_press_time
 *
 * DESCRIPTION:   Time of the last button press event.
 */
unsigned int last_button_press_time;

/*
 * NAME:          button_reads
 *
 * DESCRIPTION:   Array of <NUM_BUTTON_READS> most recent reads of button.
 *                MSB contains the oldest read; LSB contains newest read.
 *                (Unsigned Char because only 8 bits is needed).
 */
unsigned char button_reads;

/*
 * NAME:          debounced_button_state_transition
 *
 * DESCRIPTION:   Submits an event to the morse code finite state machine
 *                depending on button actions.
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
void debounced_button_state_transition(int previous_state, int event, int current_state) {
    if (previous_state == current_state) {
        // Nothing happened
        return;
    }

    switch (event) {
        case BUTTON_PRESS_EVENT:
            // Time that the putton was pressed
            last_button_press_time = current_time;
            break;
        case BUTTON_RELEASE_EVENT:
            // If time between last button press and release is greater than
            // a threshold, it is a DASH; otherwise it is a DOT.
            if ((current_time - last_button_press_time) >= DASH_DELAY_THRESHOLD_MS) {
                // DASH occured
                perform_state_transition(&morse_code_fsm, MORSE_CODE_DASH_EVENT);
            } else {
                // DOT occured
                perform_state_transition(&morse_code_fsm, MORSE_CODE_DOT_EVENT);
            }
            break;
    }
}

/*
 * NAME:          read_debounced_button
 *
 * DESCRIPTION:   Read button status and debounce. Debounce is implemented by
 *                Polling the button's status (pressed or not) every
 *                millisecond, until it is constant <NUM_BUTTON_READS> times.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void read_debounced_button(void) {
    unsigned char button_read_masked;

    // Read new button status and shift all the previous ones
    button_reads <<= 1;
    button_reads |= ~(LPC_GPIO2->FIOPIN >> 10) & 0x01;

    button_read_masked = button_reads & BUTTON_READ_MASK;

    if (button_read_masked == BUTTON_READ_MASK) {
        // Pressed
        perform_state_transition(&debounced_button_fsm, BUTTON_PRESS_EVENT);
    } else if (button_read_masked == 0) {
        // Released
        perform_state_transition(&debounced_button_fsm, BUTTON_RELEASE_EVENT);
    }
}

/*
 * NAME:          init_timer
 *
 * DESCRIPTION:   Initializes and sets up the timer to tick every 1 ms.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_timer(void) {
    LPC_TIM0->TCR = 0x02; // Reset Timer
    LPC_TIM0->TCR = 0x01; // Enable Timer
    LPC_TIM0->MR0 = 25000; // Match value  (M = 100; N = 6; F = 12MHz; CCLKSEL set to divide by 4; PCLK_TIMER0 set to divice by 4
                           // so 2 * M * F / (N * CCLKSEL_DIV_VALUE * PCLK_DIV_TIMER0 * 1000) = 2 * 100 * 12000000/(6 * 4 * 4 * 1000) = 25000
    LPC_TIM0->MCR |= 0x03; // On match, generate interrupt and reset
    NVIC_EnableIRQ(TIMER0_IRQn); // Allow for interrupts from Timer0
 }

/*
 * NAME:          TIMER0_IRQHandler
 *
 * DESCRIPTION:   Interrupt handler for TIMER0. Should fire every 1 ms.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void TIMER0_IRQHandler(void) {
    LPC_TIM0->IR |= 0x01; // Clear interrupt request

    if ((++current_time % TIME_BETWEEN_BUTTON_READS_MS) == 0) {
        // Time has passed since last button read
        read_debounced_button();
    }
}

/*
 * NAME:          init_debounced_button_fsm
 *
 * DESCRIPTION:   Initializes and sets up the debounced button finite state
 *                machine.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_debounced_button_fsm(void) {
    debounced_button_fsm.current_state = BUTTON_RELEASED_STATE;
    debounced_button_fsm.num_transitions = NUM_POSSIBLE_BUTTON_TRANSITIONS;
    debounced_button_fsm.transitions = POSSIBLE_BUTTON_TRANSITIONS;
    debounced_button_fsm.transition_function = &debounced_button_state_transition;
}

/*
 * See debounced_button.h for comments.
 */
void init_debounced_button(void) {
    LPC_PINCON->PINSEL4 &= ~(3 << 20); // P2.10 is GPIO
    LPC_GPIO2->FIODIR &= ~(1 << 10); // P2.10 is input

    button_reads = 0;

    init_debounced_button_fsm();
    init_timer();
}
