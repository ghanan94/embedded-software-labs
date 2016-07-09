/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
// #include "list.h" // This is included in tash.h already

/* Standard include. */
#include <stdio.h>

/* The ITM port is used to direct the printf() output to the serial window in
the Keil simulator IDE. */
#define mainITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define mainITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define mainDEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define mainTRCENA          0x01000000

/*
 * NAME:          NUM_TASKS
 *
 * DESCRIPTION:   Number of tasks.
 */
#define NUM_TASKS           3

/*
 * NAME:          TASK_PRIORITY
 *
 * DESCRIPTION:   Priority of each task (same for each task). The priority will
 *                be 1 step higher than an idle task.
 */
#define TASK_PRIORITY       (tskIDLE_PRIORITY + 1)
/*-----------------------------------------------------------*/

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
void task( void * );

/*
 * Redirects the printf() output to the serial window in the Keil simulator
 * IDE.
 */
int fputc( int iChar, FILE *pxNotUsed );
/*-----------------------------------------------------------*/

/* One array position is used for each task created by this demo.  The
variables in this array are set and cleared by the trace macros within
FreeRTOS, and displayed on the logic analyzer window within the Keil IDE -
the result of which being that the logic analyzer shows which task is
running when. */
unsigned long ulTaskNumber[ configEXPECTED_NO_RUNNING_TASKS ];
/*-----------------------------------------------------------*/

int main( void )
{
	// Create 4 tasks
	xTaskCreate( task, "task1", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, NULL );
	xTaskCreate( task, "task2", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, NULL );
	xTaskCreate( task, "task3", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, NULL );

	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );

	return 0;
}

void task( void *parameters )
{
	for( ;; );
}

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
