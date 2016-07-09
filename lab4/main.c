/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "list.h"

/* Standard include. */
#include <stdio.h>

#include "edf_scheduler.h"

/* The ITM port is used to direct the printf() output to the serial window in
the Keil simulator IDE. */
#define mainITM_Port8(n)     (*((volatile unsigned char *)(0xE0000000+4*n)))
#define mainITM_Port32(n)    (*((volatile unsigned long *)(0xE0000000+4*n)))
#define mainDEMCR            (*((volatile unsigned long *)(0xE000EDFC)))
#define mainTRCENA           0x01000000

#define NUM_TASKS            3

/*
 * Task priority.
 */
#define TASK_PRIORITY        (tskIDLE_PRIORITY + 1)

/*
 * Scheduler priority must be greater than tasks.
 */
#define SCHEDULER_PRIORITY   (TASK_PRIORITY + 1)

/*
 * Execution time for each task.
 */
#define TASK0_EXECUTION_TIME (1000 / portTICK_RATE_MS) // 1 s to complete
#define TASK1_EXECUTION_TIME (2000 / portTICK_RATE_MS) // 2 s to complete
#define TASK2_EXECUTION_TIME (3000 / portTICK_RATE_MS) // 3 s to complete

/*
 * Period for tasks (time between task being re-started).
 */
#define TASK0_PERIOD         (4000 / portTICK_RATE_MS) // 4 s before restarting
#define TASK1_PERIOD         (6000 / portTICK_RATE_MS) // 6 s before restarting
#define TASK2_PERIOD         (8000 / portTICK_RATE_MS) // 8 s before restarting
/*-----------------------------------------------------------*/

/*
 * Array to keep track of each task.
 */
struct tcb tcbs[NUM_TASKS];

/*
 * Array to keep track of task list items.
 */
xListItem list_items[NUM_TASKS];

/* One array position is used for each task created by this demo.  The
variables in this array are set and cleared by the trace macros within
FreeRTOS, and displayed on the logic analyzer window within the Keil IDE -
the result of which being that the logic analyzer shows which task is
running when. */
unsigned long ulTaskNumber[ configEXPECTED_NO_RUNNING_TASKS ];
/*-----------------------------------------------------------*/

/*
 * Redirects the printf() output to the serial window in the Keil simulator
 * IDE.
 */
int fputc( int iChar, FILE *pxNotUsed )
{
	/* Just to avoid compiler warnings. */
	( void ) pxNotUsed;

	if( mainDEMCR & mainTRCENA )
	{
		while( mainITM_Port32( 0 ) == 0 );
		mainITM_Port8( 0 ) = iChar;
  	}

  	return( iChar );
}

/*
 * NAME:          task
 *
 * DESCRIPTION:   A simple function to simulate a working task (endless loop).
 *
 * PARAMETERS:
 *  void *parameters
 *    - Parameters
 *
 * RETURNS:
 *  N/A
 */
void task( void *parameters )
{
	for( ;; );
}

int main( void )
{
	initialize_edf_scheduler(SCHEDULER_PRIORITY);

	// Initialize task info for each of the tasks.
	initialize_task( &list_items[0], &tcbs[0], 0, TASK0_EXECUTION_TIME, TASK0_PERIOD );
	initialize_task( &list_items[1], &tcbs[1], 1, TASK1_EXECUTION_TIME, TASK1_PERIOD );
	initialize_task( &list_items[2], &tcbs[2], 2, TASK2_EXECUTION_TIME, TASK2_PERIOD );

	// Create 3 tasks.
	xTaskCreate( task, "task0", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, &tcbs[0].handle );
	xTaskCreate( task, "task1", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, &tcbs[1].handle );
	xTaskCreate( task, "task2", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, &tcbs[2].handle );

	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );

	return 0;
}
