#include "edf_scheduler.h"
#include <stdio.h>

/*
 * ****list.h did not implement a getter for owner so implemented here.*****
 * Access macro the retrieve the ownder of the list item.  The owner of a list
 * item is the object (usually a TCB) that contains the list item.
 *
 * \page listGET_LIST_ITEM_VALUE listGET_LIST_ITEM_VALUE
 * \ingroup LinkedList
 */
#define _listGET_LIST_ITEM_OWNER( pxListItem ) (( pxListItem )->pvOwner)

/*
 * Time between scheduler ask being executed.
 */
#define SCHEDULER_PERIOD     (1000 / portTICK_RATE_MS) // 1 s before restarting

/*
 * Pointer to current task's list item.
 */
xListItem *current_task;

/*
 * List to keep track of tasks that are ready to execute.
 */
xList ready_list;

/*
 * List to keep track of tasks that are not ready to execute (wakting for a
 * wake up time to pass).
 */
xList blocked_list;

/*
 * NAME:          block_task
 *
 * DESCRIPTION:   Block a task (suspend it and add to bloacked list).
 *
 * PARAMETERS:
 *  struct task_info *task
 *    - A Task.
 *
 * RETURNS:
 *  N/A
 */
static void block_task( xListItem *task )
{
	struct tcb *tcb = ( struct tcb * )_listGET_LIST_ITEM_OWNER( current_task );
	vTaskSuspend( tcb->handle );

	// Set the wake up time.
	tcb->wake_up_time = listGET_LIST_ITEM_VALUE( task );

	// This task should be prepped to start again after a period of waking up.
	// Since the task should start again at this time, it is also the deadline
	// for the next execution.
	listSET_LIST_ITEM_VALUE( task, tcb->wake_up_time + tcb->period );

	// Reset elapsed time.
	tcb->elapsed_time = 0;

	// Add task to the blocked state.
	vListInsert( &blocked_list, task );
}

/*
 * NAME:          resume_task
 *
 * DESCRIPTION:   Resume a task and increment it's elapsed time by the
 *                scheduler period, as after the scheduler runs again, it is
 *                not guaranteed that the task may continue to run (another task
 *                may then be set to run.
 *
 * PARAMETERS:
 *  struct task_info *task
 *    - A Task.
 *
 * RETURNS:
 *  N/A
 */
static void resume_task( xListItem *task )
{
	struct tcb *tcb = ( struct tcb * )_listGET_LIST_ITEM_OWNER( current_task );

	// If the task is in a list, remove it from that list.
	// (vListRemove will get the list that the task is in and then remove it).
	vListRemove( task );

	// Increment elapsed time
	tcb->elapsed_time += SCHEDULER_PERIOD;

	vTaskResume( tcb->handle );

	printf("Task #%d\n", tcb->id);
}

/*
 * NAME:          get_next_task
 *
 * DESCRIPTION:   Get the next task that should be executed as per EDF
 *                Scheduler.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  xListItem *next_current_task
 *    - Pointer to the task that should be executed in the next slot.
 */
static xListItem* get_next_task()
{
	if ( listLIST_IS_EMPTY( &ready_list ) )
	{
		return 0;
	}

	// Must do the previous of the last item because in xLists, the last item
	// is just a marker. The xListEnd item is just a random listitem that
	// holds the highest value. The item we are looking for is right before
	// this.
	return ( xListItem * )ready_list.xListEnd.pxPrevious;
}

/*
 * NAME:          check_blocked_tasks
 *
 * DESCRIPTION:   Check the blocked list and see if any task can be unblocked.
 *
 * PARAMETERS:
 *  N/A
 *
 * RETURNS:
 *  N/A
 */
static void check_blocked_tasks()
{
	int i;
	volatile xListItem *list_item;
	portTickType tick_count;

	tick_count = xTaskGetTickCount();

	// xList stores a pointer to the tail (xListEnd), which holds a pointer
	// back to the beginning (pxNext). (Circular array/buffer).
	list_item = blocked_list.xListEnd.pxNext;

	for ( i = listCURRENT_LIST_LENGTH( &blocked_list ); i != 0; --i )
	{
		struct tcb *tcb = ( struct tcb * )_listGET_LIST_ITEM_OWNER( list_item );

		if ( tcb->wake_up_time <= tick_count )
		{
			// It is currently task's wake up time, or it has passed.
			// So put it into the ready list now.
			// Must cast to non-volatile xListItem* first
			vListRemove( ( xListItem * )list_item );
			vListInsert( &ready_list, ( xListItem * )list_item );
		}

		list_item = list_item->pxNext;
	}
}

/*
 * NAME:          edf_scheduler
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
static void edf_scheduler( void *parameters )
{
	/* Initialise xNextWakeTime - this only needs to be done once. */
	portTickType next_wake_time = xTaskGetTickCount();

	for( ;; )
	{
		printf("Scheduler [%d]\n", next_wake_time);

		// Check to see if tasks that are currently blocked should be
		// unblocked.
		check_blocked_tasks();

		if ( current_task )
		{
			struct tcb *tcb = ( struct tcb * )_listGET_LIST_ITEM_OWNER( current_task );

			if ( tcb->elapsed_time + SCHEDULER_PERIOD >= tcb->execution_time )
			{
				// Current task has completed executing.
				block_task( current_task );
			}
			else
			{
				// Task needs to continue to be done, but suspend it for now
				// so other running tasks have a change at executing aswell.
				vTaskSuspend( tcb->handle );
				vListInsert ( &ready_list, current_task );
			}
		}

		current_task = get_next_task();

		if ( current_task )
		{
			resume_task( current_task );
		}

		// Block scheduler until next period.
		vTaskDelayUntil( &next_wake_time, SCHEDULER_PERIOD );
	}
}

/*
 * Task Initializer
 */
void initialize_task( xListItem *list_item, struct tcb *tcb, unsigned int id, unsigned int execution_time, unsigned int period )
{
	tcb->id = id;
	tcb->execution_time = execution_time;
	tcb->period = period;
	tcb->elapsed_time = 0;
	tcb->wake_up_time = 0;

	// Initialize the list item before using it.
	vListInitialiseItem( list_item );

	// ListItem Owner is used to hold the tcb details.
	listSET_LIST_ITEM_OWNER( list_item, tcb );

	// ListItem Value is used to hold the time when task should be unblocked.
	listSET_LIST_ITEM_VALUE( list_item, period );

	// Add task to ready list.
	vListInsert( &ready_list, list_item );
}

/*
 * Initialize the EDF Scheduler.
 */
void initialize_edf_scheduler( unsigned int priority )
{
	xTaskCreate( edf_scheduler, "edf_scheduler", configMINIMAL_STACK_SIZE, NULL, priority, NULL );

	vListInitialise( &ready_list );
	vListInitialise( &blocked_list );
}
