#include "morse_code.h"
#include <stdlib.h>
#include "glcd.h"
#include <lpc17xx.h>

/*
 * NAME:          NUM_POSSIBLE_MORSE_CODE_TRANSITIONS
 *
 * DESCRIPTION:   Total number of Morse code pattern transitions.
 */
#define NUM_POSSIBLE_MORSE_CODE_TRANSITIONS 12

/*
 * NAME:          POSSIBLE_MORSE_CODE_TRANSITIONS
 *
 * DESCRIPTION:   Array of all possible state transitions for Morse code
 *                pattern [dot dash dash dot dash dot dot].
 */
struct transition POSSIBLE_MORSE_CODE_TRANSITIONS[] = {
    { MORSE_CODE_STAGE_0_STATE, MORSE_CODE_DOT_EVENT, MORSE_CODE_STAGE_1_STATE },   // [dot]
    { MORSE_CODE_STAGE_1_STATE, MORSE_CODE_DASH_EVENT, MORSE_CODE_STAGE_2_STATE },  // [dot dash]
    { MORSE_CODE_STAGE_2_STATE, MORSE_CODE_DOT_EVENT, MORSE_CODE_STAGE_1_STATE },   // dot dash [dot]
    { MORSE_CODE_STAGE_2_STATE, MORSE_CODE_DASH_EVENT, MORSE_CODE_STAGE_3_STATE },  // [dot dash dash]
    { MORSE_CODE_STAGE_3_STATE, MORSE_CODE_DOT_EVENT, MORSE_CODE_STAGE_4_STATE },   // [dot dash dash dot]
    { MORSE_CODE_STAGE_3_STATE, MORSE_CODE_DASH_EVENT, MORSE_CODE_STAGE_0_STATE },  // dot dash dash dash
    { MORSE_CODE_STAGE_4_STATE, MORSE_CODE_DOT_EVENT, MORSE_CODE_STAGE_1_STATE },   // dot dash dash dot [dot]
    { MORSE_CODE_STAGE_4_STATE, MORSE_CODE_DASH_EVENT, MORSE_CODE_STAGE_5_STATE },  // [dot dash dash dot dash]
    { MORSE_CODE_STAGE_5_STATE, MORSE_CODE_DOT_EVENT, MORSE_CODE_STAGE_6_STATE },   // [dot dash dash dot dash dot]
    { MORSE_CODE_STAGE_5_STATE, MORSE_CODE_DASH_EVENT, MORSE_CODE_STAGE_3_STATE },  // dot dash dash [dot dash dash]
    { MORSE_CODE_STAGE_6_STATE, MORSE_CODE_DOT_EVENT, MORSE_CODE_STAGE_7_STATE },   // [dot dash dash dot dash dot dot]
    { MORSE_CODE_STAGE_6_STATE, MORSE_CODE_DASH_EVENT, MORSE_CODE_STAGE_2_STATE }   // dot dash dash dot dash [dot dash]
};

/*
 * NAME:          CORRECT_TEXT
 *
 * DESCRIPTION:   The text for 'CORRECT'.
 */
unsigned char CORRECT_TEXT[] = "CORRECT\0";

/*
 * NAME:          DOT_TEXT
 *
 * DESCRIPTION:   The text for 'DOT'.
 */
unsigned char DOT_TEXT[] = "DOT\0";

/*
 * NAME:          DASH_TEXT
 *
 * DESCRIPTION:   The text for 'DASH'.
 */
unsigned char DASH_TEXT[] = "DASH\0";

/*
 * NAME:          led_pos
 *
 * DESCRIPTION:   Mapping of the ith led from the left to their port nums.
 *                Right most led is not included as it isn't needed
 */
unsigned char led_pos[7] = { 28, 29, 31, 2, 3, 4, 5 };

/*
 * See morse_code.h for comments.
 */
struct finite_state_machine morse_code_fsm;

/*
 * NAME:          turn_on_leds
 *
 * DESCRIPTION:   Turns on the left most leds. Number of left most leds to
 *                turn on depends on the paramter.
 *
 * PARAMETERS:
 *  int left_led_count
 *    - Number of left most leds to turn on.
 *
 * RETURNS:
 *  N/A
 */
void turn_on_leds(unsigned int left_led_count) {
    int i = 0;
    int port_1_set_mask = 0;
    int port_2_set_mask = 0;
    int port_1_clear_mask = ((1 << led_pos[0]) | (1 << led_pos[1]) | (1 << led_pos[2]));
    int port_2_clear_mask = ((1 << led_pos[3]) | (1 << led_pos[4]) | (1 << led_pos[5]) | (1 << led_pos[6]));
    int mask = 0;

    if (left_led_count > 7) {
        // Morse_code only needs 7 leds (7 stages)
        return;
    }

    for (i = 0; i < ((left_led_count > 3) ? 3 : left_led_count); ++i) {
        mask = (1 << led_pos[i]);

        port_1_set_mask |= mask;
        port_1_clear_mask &= ~mask;
    }

    for (i = 3; i < left_led_count; ++i) {
        mask = (1 << led_pos[i]);

        port_2_set_mask |= mask;
        port_2_clear_mask &= ~mask;
    }

    LPC_GPIO1->FIOSET = port_1_set_mask;
    LPC_GPIO2->FIOSET = port_2_set_mask;
    LPC_GPIO1->FIOCLR = port_1_clear_mask;
    LPC_GPIO2->FIOCLR = port_2_clear_mask;
}

/*
 * NAME:          morse_code_state_transition
 *
 * DESCRIPTION:   Updates the text on the screen depending on the event and
 *                current state. If pattern is completed (on stage 7), display
 *                'CORRECT'. Otherwise, if a dot or a dash event occured,
 *                display 'DOT' or 'DASH', respectively.
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
void morse_code_state_transition(int previous_state, int event, int current_state) {
    int leds_to_turn_on = 0;

    if ((previous_state == MORSE_CODE_STAGE_7_STATE) && (previous_state == current_state)) {
        // Previous state was the final stage in the fsm.
        // Do nothing after this unless for some reason,
        // the state changed (shouldn't happen though)
        // This prevents 'CORRECT' text from flickering on
        // button press.
        return;
    }

    switch (current_state) {
        case MORSE_CODE_STAGE_1_STATE:
            leds_to_turn_on = 1;
            break;
        case MORSE_CODE_STAGE_2_STATE:
            leds_to_turn_on = 2;
            break;
        case MORSE_CODE_STAGE_3_STATE:
            leds_to_turn_on = 3;
            break;
        case MORSE_CODE_STAGE_4_STATE:
            leds_to_turn_on = 4;
            break;
        case MORSE_CODE_STAGE_5_STATE:
            leds_to_turn_on = 5;
            break;
        case MORSE_CODE_STAGE_6_STATE:
            leds_to_turn_on = 6;
            break;
        case MORSE_CODE_STAGE_7_STATE:
            leds_to_turn_on = 7;
            break;
        default:
            leds_to_turn_on = 0;
            break;
    }

    turn_on_leds(leds_to_turn_on);

    if (current_state == MORSE_CODE_STAGE_7_STATE) {
        GLCD_Clear(White);
        GLCD_DisplayString(0, 0, 1, CORRECT_TEXT);
    } else {
        switch (event) {
            case MORSE_CODE_DOT_EVENT:
                GLCD_Clear(White);
                GLCD_DisplayString(0, 0, 1, DOT_TEXT);
                break;
            case MORSE_CODE_DASH_EVENT:
                GLCD_Clear(White);
                GLCD_DisplayString(0, 0, 1, DASH_TEXT);
                break;
        }
    }
}

/*
 * NAME:          init_leds
 *
 * DESCRIPTION:   Initializes and sets up the left most 7 leds which will
 *                display state  information.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_leds(void) {
    // Only init the left most 7 bits
    LPC_GPIO1->FIODIR |= 0xB0000000; // LEDS on PORT1 (P1.28, P1.29, P2.31)
    LPC_GPIO2->FIODIR |= 0x0000003C; // LEDS on PORT2 (P2.2, P2.3, P2.4, P2.5);
}

/*
 * See morse_code.h for comments.
 */
void init_morse_code_fsm(void) {
    morse_code_fsm.current_state = MORSE_CODE_STAGE_0_STATE;
    morse_code_fsm.num_transitions = NUM_POSSIBLE_MORSE_CODE_TRANSITIONS;
    morse_code_fsm.transitions = POSSIBLE_MORSE_CODE_TRANSITIONS;
    morse_code_fsm.transition_function = &morse_code_state_transition;

    init_leds();
}
