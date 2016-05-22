/*
 * General purpose finite state machine framework
 */
#ifndef _FSM_H
#define _FSM_H

/*
 * NAME:          transition
 *
 * DESCRIPTION:   Mapping of what the result state will be when a
 *                specific event occurs, at a specified state.
 *
 * MEMBERS:
 *  int source_state
 *    - State at the start of the transition
 *  int event
 *    - Event that occured when currently at a state
 *  int destination_state
 *    - Resulting state as a result of the event and current state
 */
struct transition {
  int source_state;
  int event;
  int destination_state;
};

/*
 * NAME:          finite_state_machine
 *
 * DESCRIPTION:   A finite state machine's data/information
 *
 * MEMBERS:
 *  int current_state
 *    - Current state of the finite state machine
 *  unsigned int num_transitions
 *    - Total number of possible state transitions
 *  struct transition *transitions
 *    - Array of all possible state transitions
 */
struct finite_state_machine {
  int current_state;
  unsigned int num_transitions;
  struct transition *transitions;
};

/*
 * NAME:          perform_state_transition
 *
 * DESCRIPTION:   Update the finite state machine's current state, given an
 *                event
 *
 * PARAMETERS:
 *  struct finite_state_machine fsm
 *    - Pointer to a finite state machine
 *  int event
 *    - An event that occured
 *
 * RETURNS:
 *  void
 */
void perform_state_transition(struct finite_state_machine *, int);

#endif
