#ifndef __EDF_SCHEDULER__
#define __EDF_SCHEDULER__

#include "FreeRTOS.h"
#include "task.h"
#include "list.h"

/*
 * Data for the EDF scheduler to function
 *
 * MEMBERS:
 *   xList ready_tasks_list
 *     - List to keep track of tasks that are ready to execute.
 *   xList blocked_tasks_list
 *     - List to keep track of tasks that are not ready to execute (wakting for a
 *       wake up time to pass).
 *   xList *current_task
 *     - Pointer to current task's list item.
 */
struct edf_scheduler_data
{
	xList ready_tasks_list;
	xList blocked_tasks_list;
	xListItem *current_task;
};

/*
 * Task Control Buffer.
 */
struct tcb
{
	unsigned int id;
	unsigned int execution_time;
	unsigned int period;
	unsigned int elapsed_time;
	unsigned int wake_up_time;
	xTaskHandle handle;
};

/*
 * NAME:          initialize_task_info
 *
 * DESCRIPTION:   Initialize task info for a task.
 *
 * PARAMETERS:
 *  xListItem *list_item
 *    - Pointer to a xListItem.
 *  struct task_info *task_info
 *    - Pointer to a task_info struct.
 *  unsigned int id
 *    - ID for a task.
 *  unsigned int execution_time
 *    - Execution time of a task.
 *  unsigned int period
 *    - Period (time between restarting) of a task.
 *  struct edf_scheduler_data *edf_scheduler_data
 *    - Pointer to struct edf_scheduler_data.
 *
 * RETURNS:
 *  N/A
 */
void initialize_task( struct edf_scheduler_data *, xListItem *, struct tcb *, unsigned int, unsigned int, unsigned int );

/*
 * NAME:          initialize_edf_scheduler
 *
 * DESCRIPTION:   Initialize the EDF Scheduler.
 *
 * PARAMETERS:
 *  unsigned int priority
 *    - Priority of the scheduler (should be the highest priority task).
 *  struct edf_scheduler_data *edf_scheduler_data
 *    - Pointer to struct edf_scheduler_data.
 *
 * RETURNS:
 *  N/A
 */
void initialize_edf_scheduler( struct edf_scheduler_data *, unsigned int );

#endif // __EDF_SCHEDULER__
