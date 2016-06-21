/*
 * Debounced button
 */
#ifndef _DEBOUNCED_BUTTONS_H
#define _DEBOUNCED_BUTTONS_H

#include <fsm/fsm.h>

/*
 * NAME:          BUTTON_STATES
 *
 * DESCRIPTION:   Enum for the various states of a button.
 *
 * ENUMERATORS:
 *  BUTTON_PRESSED_STATE
 *    - Button is pressed.
 *  BUTTON_RELEASED_STATE
 *    - Button is released.
 */
enum BUTTON_STATES {
    BUTTON_PRESSED_STATE,
    BUTTON_RELEASED_STATE
};

/*
 * NAME:          BUTTON_EVENTS
 *
 * DESCRIPTION:   Enum for the various events that can occur with a button.
 *
 * ENUMERATORS:
 *  BUTTON_PRESS_EVENT
 *    - Button has been pressed.
 *  BUTTON_RELEASE_EVENT
 *    - Button has been released.
 */
enum BUTTON_EVENTS {
    BUTTON_PRESS_EVENT,
    BUTTON_RELEASE_EVENT
};

/*
 * NAME:          init_debounced_buttons
 *
 * DESCRIPTION:   Initializes and sets up the buttons (Joystick) which will be
 *                debounced.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_debounced_buttons(void);

#endif
