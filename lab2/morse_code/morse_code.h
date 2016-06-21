/*
 * Match the Morse-code pattern specified
 * dot-dash-dash-dot-dash-dot-dot
 */
#ifndef _MORSE_CODE_H
#define _MORSE_CODE_H

#include <fsm/fsm.h>

/*
 * NAME:          MORSE_CODE_STATES
 *
 * DESCRIPTION:   Enum for the various states of the specified pattern
 *                dot-dash-dash-dot-dash-dot-dot.
 *
 * ENUMERATORS:
 *  MORSE_CODE_STAGE_0_STATE
 *    - Nothing has happened yet (system just started and no relevant event
 *      has occured yet) or the sequence of the events before this state
 *      is not a part of the specified pattern.
 *  MORSE_CODE_STAGE_1_STATE
 *    - The first event (dot) of the pattern has occured.
 *  MORSE_CODE_STAGE_2_STATE
 *    - The second event (dash) of the pattern has occured.
 *  MORSE_CODE_STAGE_3_STATE
 *    - The third event (dash) of the pattern has occured.
 *  MORSE_CODE_STAGE_4_STATE
 *    -The fourth event (dot) of the pattern has occured.
 *  MORSE_CODE_STAGE_5_STATE
 *    - The fifth event (dash) of the pattern has occured.
 *  MORSE_CODE_STAGE_6_STATE
 *    - The sixth event (dot) of the pattern has occured.
 *  MORSE_CODE_STAGE_7_STATE
 *    - The seventh (and last) event (dot) of the pattern has occured.
        The full pattern has been completed at this state.
 */
enum MORSE_CODE_STATES {
    MORSE_CODE_STAGE_0_STATE,
    MORSE_CODE_STAGE_1_STATE,
    MORSE_CODE_STAGE_2_STATE,
    MORSE_CODE_STAGE_3_STATE,
    MORSE_CODE_STAGE_4_STATE,
    MORSE_CODE_STAGE_5_STATE,
    MORSE_CODE_STAGE_6_STATE,
    MORSE_CODE_STAGE_7_STATE
};

/*
 * NAME:          MORSE_CODE_EVENTS
 *
 * DESCRIPTION:   Enum for the various events that can occur with Morse Code.
 *
 * ENUMERATORS:
 *  MORSE_CODE_DOT_EVENT
 *    - A single press event (dot).
 *  MORSE_CODE_DASH_EVENT
 *    - A long press event (dash).
 */
enum MORSE_CODE_EVENTS {
    MORSE_CODE_DOT_EVENT,
    MORSE_CODE_DASH_EVENT
};

/*
 * NAME:          morse_code_fsm
 *
 * DESCRIPTION:   Finite state machine for the Morse code pattern specified.
 */
extern struct finite_state_machine morse_code_fsm;

/*
 * NAME:          init_morse_code_fsm
 *
 * DESCRIPTION:   Initializes and sets up the Morse code finite state machine.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_morse_code_fsm(void);

#endif
