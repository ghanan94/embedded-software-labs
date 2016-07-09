/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
// #include "list.h" // This is included in tash.h already

/* Standard include. */
#include <stdio.h>

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
#define SCHEDULER_PERIOD     (1000 / portTICK_RATE_MS) // 1 s before restarting
/*-----------------------------------------------------------*/

struct task_info {
	unsigned int id;
	unsigned int execution_time;
	unsigned int period;
	unsigned int elapsed_time;
	unsigned int wake_up_time;
	xTaskHandle handle;
};
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
 * NAME:          scheduler
 *
 * DESCRIPTION:   EDF Scheduler.
 *
 * PARAMETERS:
 *  void *parameters
 *    - Parameters
 *
 * RETURNS:
 *  N/A
 */
void scheduler( void * );

/*
 * NAME:          initialize_task_info
 *
 * DESCRIPTION:   Initialize task info for a task.
 *
 * PARAMETERS:
 *  struct task_info *task_info
 *    - Pointer to a task_info struct.
 *  unsigned int id
 *    - ID for a task.
 *  unsigned int execution_time
 *    - Execution time of a task.
 *  unsigned int period
 *    - Period (time between restarting) of a task.
 *
 * RETURNS:
 *  N/A
 */
void initialize_task_info( struct task_info *, unsigned int, unsigned int, unsigned int);

/*
 * Redirects the printf() output to the serial window in the Keil simulator
 * IDE.
 */
int fputc( int iChar, FILE *pxNotUsed );
/*-----------------------------------------------------------*/

/*
 * Array to keep track of each task
 */
struct task_info task_infos[NUM_TASKS];

/* One array position is used for each task created by this demo.  The
variables in this array are set and cleared by the trace macros within
FreeRTOS, and displayed on the logic analyzer window within the Keil IDE -
the result of which being that the logic analyzer shows which task is
running when. */
unsigned long ulTaskNumber[ configEXPECTED_NO_RUNNING_TASKS ];
/*-----------------------------------------------------------*/

int main( void )
{
	// Initialize task info for each of the tasks
	initialize_task_info(&task_infos[0], 0, TASK0_EXECUTION_TIME, TASK0_PERIOD);
	initialize_task_info(&task_infos[1], 1, TASK1_EXECUTION_TIME, TASK1_PERIOD);
	initialize_task_info(&task_infos[2], 2, TASK2_EXECUTION_TIME, TASK2_PERIOD);

	// Create 3 tasks and a scheduler task
	xTaskCreate( scheduler, "scheduler", configMINIMAL_STACK_SIZE, NULL, SCHEDULER_PRIORITY, NULL );
	xTaskCreate( task, "task0", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, &task_infos[0].handle );
	xTaskCreate( task, "task1", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, &task_infos[1].handle );
	xTaskCreate( task, "task2", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, &task_infos[2].handle );

	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );

	return 0;
}

void initialize_task_info( struct task_info *task_info, unsigned int id, unsigned int execution_time, unsigned int period)
{
	task_info->id = id;
	task_info->execution_time = execution_time;
	task_info->period = period;
	task_info->elapsed_time = 0;
	task_info->wake_up_time = 0;
}

void scheduler( void *parameters )
{
	/* Initialise xNextWakeTime - this only needs to be done once. */
	portTickType next_wake_time = xTaskGetTickCount();

	for( ;; )
	{

		/* Place this task in the blocked state until it is time to run again.
		The block time is specified in ticks, the constant used converts ticks
		to ms.  While in the Blocked state this task will not consume any CPU
		time. */
		vTaskDelayUntil( &next_wake_time, SCHEDULER_PERIOD );
	}
}

void task( void *parameters )
{
	for( ;; );
}

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
