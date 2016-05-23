/*
 * Thermostat
 */
#ifndef _THERMOSTAT_H
#define _THERMOSTAT_H

#include <fsm/fsm.h>

/*
 * NAME:          THERMOSTAT_EVENTS
 *
 * DESCRIPTION:   Enum for the various events that can occur with thermostat.
 *
 * ENUMERATORS:
 *  THERMOSTAT_IDLE_STATE
 *    - Thermostat is idle (actual temperature is okay).
 *  THERMOSTAT_HEATING_STATE
 *    - Thermostat is heating (actual temperature is too cool).
 *  THERMOSTAT_COOLING_STATE
 *    - Thermostat is cooling (actual temperature is too hot).
 */
enum THERMOSTAT_STATES {
    THERMOSTAT_IDLE_STATE,
    THERMOSTAT_HEATING_STATE,
    THERMOSTAT_COOLING_STATE
};

/*
 * NAME:          THERMOSTAT_EVENTS
 *
 * DESCRIPTION:   Enum for the various events that can occur with thermostat.
 *
 * ENUMERATORS:
 *  THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_OKAY_EVENT
 *    - Actual temperature is good (within 1 degree of set temperature).
 *  THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_HOT_EVENT
 *    - Actual temperature is hot (more than 1 degree of set temperature).
 *  THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_COOL_EVENT
 *    - Actual temperature is cool (less than 1 degree of set temperature).
 */
enum THERMOSTAT_EVENTS {
    THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_OKAY_EVENT,
    THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_HOT_EVENT,
    THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_COOL_EVENT
};

/*
 * NAME:          thermostat_fsm
 *
 * DESCRIPTION:   Finite state machine for the thermostat.
 */
extern struct finite_state_machine thermostat_fsm;

/*
 * NAME:          init_thermostat
 *
 * DESCRIPTION:   Initializes and sets up the thermostat finite state machine.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_thermostat(void);

#endif
