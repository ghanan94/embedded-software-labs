#include "thermostat.h"

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
 * See thermostat.h for comments.
 */
struct finite_state_machine thermostat_fsm;

/*
 * See thermostat.h for comments.
 */
void init_thermostat(void) {
    thermostat_fsm.current_state = THERMOSTAT_IDLE_STATE;
    thermostat_fsm.num_transitions = NUM_POSSIBLE_THERMOSTAT_TRANSITIONS;
    thermostat_fsm.transitions = POSSIBLE_THERMOSTAT_TRANSITIONS;
}
