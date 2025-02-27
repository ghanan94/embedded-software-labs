#include "thermostat.h"
#include "glcd.h"
#include <lpc17xx.h>

/*
 * NAME:          TIME_BETWEEN_TEMPERATURE_READS_MS
 *
 * DESCRIPTION:   Time in milliseconds between reading button status.
 */
#define TIME_BETWEEN_TEMPERATURE_READS_MS 20

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
    { THERMOSTAT_HEATING_STATE, THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_OKAY_EVENT, THERMOSTAT_IDLE_STATE },
    { THERMOSTAT_COOLING_STATE, THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_COOL_EVENT, THERMOSTAT_HEATING_STATE },
    { THERMOSTAT_COOLING_STATE, THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_OKAY_EVENT, THERMOSTAT_IDLE_STATE }
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
 * NAME:          set_temperature
 *
 * DESCRIPTION:   The temperature the thermostat is set to.
 */
int set_temperature;

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
 * NAME:          TIMER0_IRQHandler
 *
 * DESCRIPTION:   Interrupt handler for TIMER0. Should fire every time a
 *                temperature read should occur.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void TIMER1_IRQHandler(void) {
    LPC_TIM1->IR |= 0x01; // Clear interrupt request

    // Start the A/D conversion to get
    // actual temperature data (potentiometer)
    LPC_ADC->ADCR |= (1 << 24);
}

/*
 * NAME:          ADC_IRQHandler
 *
 * DESCRIPTION:   Interrupt handler for ADC. Should fire when an A/D
 *                conversion is complete.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void ADC_IRQHandler(void) {
    int actual_temperature = ((LPC_ADC->ADGDR >> 4) & 0xFFF) / 40; // read ADC Result
	// ADC returns 0x0 to 0xFFF (4096) so make it so system only in temperatures beween 0 and 100

    if (actual_temperature > set_temperature) {
        // TOO HOT
        transition_state(&thermostat_fsm, THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_HOT_EVENT);
    } else if (actual_temperature < set_temperature) {
        // TOO COLD
        transition_state(&thermostat_fsm, THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_COOL_EVENT);
    } else {
        // OKAY
        transition_state(&thermostat_fsm, THERMOSTAT_ACTUAL_TEMPERATURE_SENSED_OKAY_EVENT);
    }
}

/*
 * NAME:          init_timer
 *
 * DESCRIPTION:   Initializes and sets up the timer to tick every
 *                <TIME_BETWEEN_TEMPERATURE_READS_MS> ms.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
static void init_timer(void) {
    LPC_TIM1->TCR = 0x02; // Reset Timer
    LPC_TIM1->TCR = 0x01; // Enable Timer
    // Match value of 25000 means the timer's ISR will tick every 1ms
    // given below parameters.
    //(M = 100; N = 6; F = 12MHz; CCLKSEL set to divide by 4; PCLK_TIMER0 set to divice by 4
    // so 2 * M * F / (N * CCLKSEL_DIV_VALUE * PCLK_DIV_TIMER0 * 1000) = 2 * 100 * 12000000/(6 * 4 * 4 * 1000) = 25000
    // Multiplying above value by TIME_BETWEEN_TEMPERATURE_READS_MS allows us to only
    // have to call the ISR when a read is supposed to occur,
    // instead of every time. At worse case, this occurs every 1 ms.
    LPC_TIM1->MR0 = 25000 * TIME_BETWEEN_TEMPERATURE_READS_MS;
    LPC_TIM1->MCR |= 0x03; // On match, generate interrupt and reset
    NVIC_EnableIRQ(TIMER1_IRQn); // Allow for interrupts from Timer0
}

/*
 * NAME:          init_adc
 *
 * DESCRIPTION:   Initializes and sets up the adc to read the potentiometer.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
void init_adc(void) {
    LPC_PINCON->PINSEL1 &= ~(3 << 18);
    LPC_PINCON->PINSEL1 |= (1 << 18); // P0.25 is AD0.2

    LPC_SC->PCONP |= (1 << 12); // Enable power to ADC block

    LPC_ADC->ADCR = (1 << 2) | // select AD0.2 pin
                    (4 << 8) | // ADC clock is 25Mhz/5
                    (1 << 21); // enable ADC

    LPC_ADC->ADINTEN = (1 << 2); // enable interrupt when conversion is
                                 // complete for analog input 2
    NVIC_EnableIRQ(ADC_IRQn);
}

/*
 * See thermostat.h for comments.
 */
void init_thermostat(void) {
    set_temperature = 24;

    thermostat_fsm.current_state = THERMOSTAT_IDLE_STATE;
    thermostat_fsm.num_transitions = NUM_POSSIBLE_THERMOSTAT_TRANSITIONS;
    thermostat_fsm.transitions = POSSIBLE_THERMOSTAT_TRANSITIONS;
    thermostat_fsm.transition_function = &thermostat_state_transition;

    init_adc();
    init_timer();
}

/*
 * See thermostat.h for comments.
 */
void increase_thermostat_set_temperature(void) {
  ++set_temperature;
}

/*
 * See thermostat.h for comments.
 */
void decrease_thermostat_set_temperature(void) {
  --set_temperature;
}
