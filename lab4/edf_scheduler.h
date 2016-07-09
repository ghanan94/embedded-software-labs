#ifndef __EDF_SCHEDULER__
#define __EDF_SCHEDULER__

#include "FreeRTOS.h"
#include "task.h"
#include "list.h"

/*
 * Task Control Buffer.
 */
struct tcb {
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
 *
 * RETURNS:
 *  N/A
 */
void initialize_task( xListItem *, struct tcb *, unsigned int, unsigned int, unsigned int );


/*
 * NAME:          initialize_edf_scheduler
 *
 * DESCRIPTION:   Initialize the EDF Scheduler.
 *
 * PARAMETERS:
 *  unsigned int priority
 *    - Priority of the scheduler (should be the highest priority task).
 *
 * RETURNS:
 *  N/A
 */
void initialize_edf_scheduler( unsigned int );

#endif // __EDF_SCHEDULER__
