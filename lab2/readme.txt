The general purpose finite state machine is implemented as a struct that holds the fsm's current state, a (pointer to an) array of all state transitions that can occur and a (pointer to a) function that is to be executed on state transitioning. When an event occurs for an fsm, the transition_state function will update the fsm's state with the new state and execute the function that must be callled on state transitions.
Button debouncing is done by constantly polling the button's state approximately every 5 milliseconds. The 5 most recent button states are recorded and if they are all exactly the same (all 1's or all 0's), the button is considered debounced and its status is updated and any action that needs to be done on a button press or release is done. Each button is also implemented with a finite state machine that keeps track of the button's current state and any events that occured (releasing or pressing).
For the morse_code portion of this lab, a finite state machine is created with a current state of 0. There are 7 total states with 7 being the final state (CORRECT). each state inbetween is reached when the pattern up to that point has been input. The states and state transitions are availabel below:
State0 = No pattern matched so far
State1 = Dot
State2 = Dot Dash
State 3 = Dot Dash Dash
State4 = Dot Dash Dash Dot
State5 = Dot Dash Dash Dot Dash
State6 = Dot Dash Dash Dot Dash Dot
State7 = Dot Dash Dash Dot Dash Dot Dash

State0->Dot->State1
State1->Dash->State2
State2->Dot->State1
State2->Dash->State3
State3->Dot->State4
State3->Dash->State0
State4->Dot->State1
State4->Dash->State5
State5->Dot->State6
State5->Dash->State3
State6->Dot->State7
State6->Dash->State2

For the thermostat portion of the lab, a finite state machine is created with a current state of IDLE. There are a total of 3 states. The states are as follows:
IDLE, HEATING, COOLING
The state transitions are as follows:
IDLE->TEMPERATURE_SENSED_HOT->COOLING
IDLE->TEMPERATURE_SENSED_COLD->HEATING
HEATING->TEMPERATURE_SENSED_HOT->COOLING
HEATING->TEMPERATURE_SENSED_OKAY->IDLE
COOLING->TEMPERATURE_SENSED_COLD->HEATING
COOLING->TEMPERATURE_SENSED_OKAY->IDLE
With this portion, along with the periodic reading of the button states, there is also a periodoc read of the actual temperature (potentiometer). On reach read, the set and actual temperatures are compared, and an event is sent to the thermostat state machine depending on the comparision (currently too hot, too cold, or temperature is just right).
