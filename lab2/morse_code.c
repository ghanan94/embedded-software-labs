#include "morse_code.h"
#include "fsm.h"
#include <stdlib.h>

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
 * See morse_code.h for comments.
 */
struct finite_state_machine *morse_code_fsm;

/*
 * See morse_code.h for comments.
 */
void init_morse_code_fsm(void) {

    morse_code_fsm = (struct finite_state_machine *) malloc(sizeof(struct finite_state_machine));

    morse_code_fsm->current_state = MORSE_CODE_STAGE_0_STATE;
    morse_code_fsm->num_transitions = NUM_POSSIBLE_MORSE_CODE_TRANSITIONS;
    morse_code_fsm->transitions = POSSIBLE_MORSE_CODE_TRANSITIONS;
}
