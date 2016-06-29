Strict Scheduler:

The strict scheduler is setup so that an INT0 interrupt (on EINT3) is allowed at max once every 10 seconds.

For the strict scheduler, an interrupt on EINT3 is setup (specifically port P2.10). 

On every button press, the EINT3 ISR will be called so long as interrupts on EINT3 is enabled. The EINT3 ISR will turn on an LED (P1.28), disable EINT3 interrupts, and enable TIMER0 (used to re-enable EINT3 interrupts after 10 s) and TIMER1 (used to turn off led after 200 ms).

TIMER0 will call its ISR after 10 s of being enabled (the configured minimum inter-arrival time). TIMER0 is setup so that after a match with the counter, it will reset then stop counting (disable itself). The TIMER0 ISR will re-enable EINT3 interrupts. Before re-enabling the EINT3 interrupt however, any current pending interrupts on EINT3 is disabled first to make sure that any invalid/disallowed interrupts are not lingering.

TIMER1 will call its ISR after 200 ms of being enabled (the configured LED on time). TIMER1 is setup so that after a match with the counter, it will reset then stop counting (disable itself). The TIMER1 ISR will turn off the LED (P1.28).


Bursty Scheduler:

The bursty scheduler is setup so that at max, 3 INT0 interrupts (on EINT3) is allowed at max within a 10 second time frame.

For the bursty scheduler, an interrupt on EINT3 is setup (specifically port P2.10).

On every button press, the EINT3 ISR will be called so long as interrupts on EINT3 is enabled. 

In this scenario, button debouncing was needed. For button debouncing, a minimum of 500 ms had to have elapsed between doing the work in the ISR, if less than 500 ms has ellapsed, simply do nothing. TIMER1 is used to keep track of the time which is primarily used to see how long the current button status is table for.

If the button press was considered valid (not bouncing), an LED (P1.28) is turned on, and a variable to keep track of time left for the led to be on for (200 ms) is set. The number of interrupts in the current time frame is then incremented as well. If the number of interrupts in the current time frame is now equal to 3 (the configured maximum interrupts per burst), interrupts are disabled on EINT3. If this is the first interrupt of the burst, TIMER0 (used to reset EINT3 interrupt count per burst frame after 10 s) is enabled.

TIMER0 will call its ISR after 10 s of being enabled (the configured burst time frame). TIMER0 is setup so that after a match with the counter, it will reset then stop counting (disable itself). The TIMER0 ISR will re-enable EINT3 interrupts and reset the interrupt count in the current burst frame. Before re-enabling the EINT3 interrupt however, any current pending interrupts on EINT3 is disabled first to make sure that any invalid/disallowed interrupts are not lingering.

TIMER1 is used to keep track of time. On every tick (TIMER1 ticks every 100ms), the time left for the LED to be on is decremented if it is greater than 0. Once the time left is equal to 0, the LED is turned off. TIMER1 also increments a global variable that is used to keep track of time. This is used primarily for button debouncing (button status must be stable for 500 ms).
