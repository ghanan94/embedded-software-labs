#include "thermostat.h"
#include "glcd.h"

/*
 * NAME:          NUM_POSSIBLE_THERMOSTAT_TRANSITIONS
 *
 * DESCRIPTION:   Total number of thermostat transitions.
 */
#define NUM_POSSIBLE_THERMOSTAT_TRANSITIONS 6

/*
 * NAME:          POSSIBLE_THERMOSTAT_TRANSITIONS
 *
 * DESCRIPTION:   Array of all possible state transitions for thermostat.
 */
struct transition POSSIBLE_THERMOSTAT_TRANSITIONS[] = {
    { THERMOSTAT_IDLE_STATE, THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_HOT_EVENT, THERMOSTAT_COOLING_STATE },
    { THERMOSTAT_IDLE_STATE, THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_COOL_EVENT, THERMOSTAT_HEATING_STATE },
    { THERMOSTAT_HEATING_STATE, THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_HOT_EVENT, THERMOSTAT_COOLING_STATE },
    { THERMOSTAT_COOLING_STATE, THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_COOL_EVENT, THERMOSTAT_HEATING_STATE }
};

/*
 * NAME:          IDLE_TEXT
 *
 * DESCRIPTION:   The text for IDLE state.
 */
unsigned char IDLE_TEXT[] = "IDLE\0";

/*
 * NAME:          HEATING_TEXT
 *
 * DESCRIPTION:   The text for HEATING state.
 */
unsigned char HEATING_TEXT[] = "HEATING\0";

/*
 * NAME:          COOLING_TEXT
 *
 * DESCRIPTION:   The text for COOLING state.
 */
unsigned char COOLING_TEXT[] = "COOLING\0";

/*
 * See thermostat.h for comments.
 */
struct finite_state_machine thermostat_fsm;

/*
 * NAME:          thermostat_state_transition
 *
 * DESCRIPTION:   Do something when thermostat state changes.
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
void thermostat_state_transition(int previous_state, int event, int current_state) {
    if (previous_state == current_state) {
        // Nothing happened
        return;
    }

    switch (current_state) {
        case THERMOSTAT_IDLE_STATE:
            GLCD_Clear(White);
            GLCD_DisplayString(0, 0, 1, IDLE_TEXT);
            break;
        case THERMOSTAT_HEATING_STATE:
            GLCD_Clear(Red);
            GLCD_DisplayString(0, 0, 1, HEATING_TEXT);
            break;
        case THERMOSTAT_COOLING_STATE:
            GLCD_Clear(Blue);
            GLCD_DisplayString(0, 0, 1, COOLING_TEXT);
            break;
    }
}

/*
 * See thermostat.h for comments.
 */
void init_thermostat(void) {
    thermostat_fsm.current_state = THERMOSTAT_IDLE_STATE;
    thermostat_fsm.num_transitions = NUM_POSSIBLE_THERMOSTAT_TRANSITIONS;
    thermostat_fsm.transitions = POSSIBLE_THERMOSTAT_TRANSITIONS;
    thermostat_fsm.transition_function = &thermostat_state_transition;
}
