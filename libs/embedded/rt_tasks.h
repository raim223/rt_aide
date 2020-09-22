#ifndef _RT_TASK_H_
#define _RT_TASK_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
/*****************************************************************************/
/* RT_TASKS */
/*****************************************************************************/
#include "embdCOMMON.h"

#ifdef _XENOMAI_TASKS_
	#include <alchemy/task.h> 
	#include <alchemy/timer.h> 
	#define printf rt_printf
#else
	#include "rt_posix_task.h"
	#define RT_TASK PT_TASK
	#define SRTIME PRTIME
	#define RTIME PRTIME
	#define TM_NOW TMR_NOW
	#define rt_timer_read pt_timer_read
	#define rt_timer_spin pt_timer_spin
	#define rt_timer_ns2ticks pt_timer_ns2ticks
#endif
/*****************************************************************************/
/* Real-time Task and Timers */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
int	aide_create_rt_task(RT_TASK *task, const char *name, int prio);
int	aide_create_nrt_task(RT_TASK *task, const char *name);
int aide_set_task_period(RT_TASK *task, RTIME period);
int aide_start_task(int enable, RT_TASK *task, void (*entry)(void *arg), void* arg);
int aide_start_task_noarg(int enable, RT_TASK *task, void (*entry)(void *arg));
int aide_wait_period(unsigned long *overruns_cnt);
int aide_suspend_task(RT_TASK* task);
int aide_delete_task(RT_TASK* task);
#ifdef __cplusplus
}
#endif 


#endif //_RT_TASKS_H_