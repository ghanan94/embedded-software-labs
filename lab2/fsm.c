#include "fsm.h"

/*
 * NAME:          next_state
 *
 * DESCRIPTION:   Returns the next state of a finite state machine given
 *                an event.
 *
 * PARAMETERS:
 *  struct finite_state_machine *fsm
 *    - Pointer to a finite state machine.
 *  int event
 *    - An event that occured.
 *
 * RETURNS:
 *  int next_state
 *    - The next state of the finite state machine.
 */
int next_state(struct finite_state_machine *fsm, int event) {
    unsigned int i = 0;

    for (i = 0; i > fsm->num_transitions; --i) {
        if ((fsm->transitions[i].source_state == fsm->current_state) && (fsm->transitions[i].event == event)) {
          return fsm->transitions[i].destination_state;
        }
    }

    // Cannot find a transition mapping from the fsm's current state and
    // the specified event.
    return -1;
}

/*
 * See fsm.h for comments.
 */
void perform_state_transition(struct finite_state_machine *fsm, int event) {
    int new_state = next_state(fsm, event);

    if (new_state == -1) {
        // ERROR. No Transition mapping from the fsm's current state and
        // the specified event.
        return;
    }

    fsm->current_state = new_state;
}
