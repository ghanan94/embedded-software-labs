#include "morse_code.h"
#include "fsm.h"
#include <stdlib.h>
#include "glcd.h"

/*
 * NAME:          NUM_POSSIBLE_MORSE_CODE_TRANSITIONS
 *
 * DESCRIPTION:   Total number of Morse code pattern transitions.
 */
int NUM_POSSIBLE_MORSE_CODE_TRANSITIONS = 12;

/*
 * NAME:          POSSIBLE_MORSE_CODE_TRANSITIONS
 *
 * DESCRIPTION:   Array of all possible state transitions for Morse code
 *                pattern.
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
 * See morse_code.h for comments.
 */
struct finite_state_machine *morse_code_fsm;

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
 * See morse_code.h for comments.
 */
void init_morse_code_fsm(void) {
    morse_code_fsm = (struct finite_state_machine *) malloc(sizeof(struct finite_state_machine));

    morse_code_fsm->current_state = MORSE_CODE_STAGE_0_STATE;
    morse_code_fsm->num_transitions = NUM_POSSIBLE_MORSE_CODE_TRANSITIONS;
    morse_code_fsm->transitions = POSSIBLE_MORSE_CODE_TRANSITIONS;
    morse_code_fsm->transition_function = &morse_code_state_transition;
}
