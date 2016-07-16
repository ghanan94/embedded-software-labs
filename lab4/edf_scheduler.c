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
#define _listGET_LIST_ITEM_OWNER( pxListItem ) ( ( pxListItem )->pvOwner )

/*
 * Time between scheduler ask being executed.
 */
#define SCHEDULER_PERIOD     ( 1000 / portTICK_RATE_MS ) // 1 s before restarting

/*
 * NAME:          block_task
 *
 * DESCRIPTION:   Block a task (suspend it and add to bloacked list).
 *
 * PARAMETERS:
 *   struct edf_scheduler_data *edf_scheduler_data
 *     - Pointer to struct edf_scheduler_data.
 *  struct task_info *task
 *    - A Task.
 *
 * RETURNS:
 *  N/A
 */
static void block_task( struct edf_scheduler_data *edf_scheduler_data, xListItem *task )
{
	struct tcb *tcb = ( struct tcb * )_listGET_LIST_ITEM_OWNER( task );
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
	vListInsert( &edf_scheduler_data->blocked_tasks_list, task );
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
	struct tcb *tcb = ( struct tcb * )_listGET_LIST_ITEM_OWNER( task );

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
 *   struct edf_scheduler_data *edf_scheduler_data
 *     - Pointer to struct edf_scheduler_data.
 *
 * RETURNS:
 *  xListItem *
 *    - Pointer to the task that should be executed in the next slot.
 */
static xListItem* get_next_task( struct edf_scheduler_data *edf_scheduler_data )
{
	int i;
	volatile xListItem *list_item;
	xListItem *next_list_item;

	if ( listLIST_IS_EMPTY( &edf_scheduler_data->ready_tasks_list ) )
	{
		return 0;
	}

	// xList stores a pointer to the tail (xListEnd), which holds a pointer
	// back to the beginning (pxNext). (Circular array/buffer).
	list_item = edf_scheduler_data->ready_tasks_list.xListEnd.pxNext;
	next_list_item = ( xListItem * )list_item;

	// xList is sorted in ascending order by list item Value, so we only
	// neex to check the first item. However, if the first item and second item
	// have the same deadline, use the order T1 > T2 > T3 to break the tie.
	for ( i = listCURRENT_LIST_LENGTH( &edf_scheduler_data->ready_tasks_list ); i != 0; --i )
	{
		if ( listGET_LIST_ITEM_VALUE( list_item ) == listGET_LIST_ITEM_VALUE( next_list_item ) )
		{
			struct tcb *tcb_li = ( struct tcb * )_listGET_LIST_ITEM_OWNER( list_item );
			struct tcb *tcb_nli = ( struct tcb * )_listGET_LIST_ITEM_OWNER( next_list_item );

			if ( tcb_li->id < tcb_nli->id )
			{
				next_list_item = ( xListItem * )list_item;
			}

		}
		else
		{
			// Once the value changes, we know the value increased, so we
			// break and do the task with the earliest deadline
			break;
		}

		list_item = list_item->pxNext;
	}

	return next_list_item;
}

/*
 * NAME:          check_blocked_tasks
 *
 * DESCRIPTION:   Check the blocked list and see if any task can be unblocked.
 *
 * PARAMETERS:
 *   struct edf_scheduler_data *edf_scheduler_data
 *     - Pointer to struct edf_scheduler_data.
 *
 * RETURNS:
 *  N/A
 */
static void check_blocked_tasks( struct edf_scheduler_data *edf_scheduler_data )
{
	int i;
	volatile xListItem *list_item;
	portTickType tick_count;

	tick_count = xTaskGetTickCount();

	// xList stores a pointer to the tail (xListEnd), which holds a pointer
	// back to the beginning (pxNext). (Circular array/buffer).
	list_item = edf_scheduler_data->blocked_tasks_list.xListEnd.pxNext;

	for ( i = listCURRENT_LIST_LENGTH( &edf_scheduler_data->blocked_tasks_list ); i != 0; --i )
	{
		struct tcb *tcb = ( struct tcb * )_listGET_LIST_ITEM_OWNER( list_item );

		if ( tcb->wake_up_time <= tick_count )
		{
			// It is currently task's wake up time, or it has passed.
			// So put it into the ready list now.
			// Must cast to non-volatile xListItem* first
			vListRemove( ( xListItem * )list_item );
			vListInsert( &edf_scheduler_data->ready_tasks_list, ( xListItem * )list_item );
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
	struct edf_scheduler_data *edf_scheduler_data = ( struct edf_scheduler_data *)parameters;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	portTickType next_wake_time = xTaskGetTickCount();

	for( ;; )
	{
		printf("Scheduler [%d]\n", next_wake_time);

		// Check to see if tasks that are currently blocked should be
		// unblocked.
		check_blocked_tasks( edf_scheduler_data );

		if ( edf_scheduler_data->current_task )
		{
			struct tcb *tcb = ( struct tcb * )_listGET_LIST_ITEM_OWNER( edf_scheduler_data->current_task );

			if ( tcb->elapsed_time  >= tcb->execution_time )
			{
				// Current task has completed executing.
				block_task( edf_scheduler_data, edf_scheduler_data->current_task );
			}
			else
			{
				// Task needs to continue to be done, but suspend it for now
				// so other running tasks have a change at executing aswell.
				vTaskSuspend( tcb->handle );
				vListInsert ( &edf_scheduler_data->ready_tasks_list, edf_scheduler_data->current_task );
			}
		}

		edf_scheduler_data->current_task = get_next_task( edf_scheduler_data );

		if ( edf_scheduler_data->current_task )
		{
			resume_task( edf_scheduler_data->current_task );
		}

		// Block scheduler until next period.
		vTaskDelayUntil( &next_wake_time, SCHEDULER_PERIOD );
	}
}

/*
 * Task Initializer
 */
void initialize_task( struct edf_scheduler_data *edf_scheduler_data, xListItem *list_item, struct tcb *tcb, unsigned int id, unsigned int execution_time, unsigned int period )
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
	vListInsert( &edf_scheduler_data->ready_tasks_list, list_item );

	// Initially, task should be in a suspended state.
	vTaskSuspend( tcb->handle );
}

/*
 * Initialize the EDF Scheduler.
 */
void initialize_edf_scheduler( struct edf_scheduler_data *edf_scheduler_data, unsigned int priority )
{
	xTaskCreate( edf_scheduler, "edf_scheduler", configMINIMAL_STACK_SIZE, ( void * )edf_scheduler_data, priority, NULL );

	vListInitialise( &edf_scheduler_data->ready_tasks_list );
	vListInitialise( &edf_scheduler_data->blocked_tasks_list );
}
