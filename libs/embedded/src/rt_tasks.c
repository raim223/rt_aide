#include <rt_tasks.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/*****************************************************************************/
#define DEFAULT_TASK_STKSIZE 0
int _create_rt_task(RT_TASK *task, char *name, int stksize, int prio);
int _set_rt_task_period(RT_TASK *task, SRTIME period);
/*
****************************************************************************/
int _create_rt_task(RT_TASK *task, char *name, int stksize, int prio) {
	int ret = -1;
	char str[1024]={0,};

#ifdef _XENOMAI_TASKS_
	ret = rt_task_create(task, name, stksize*1024, prio, 0);
#else
	if (prio == 0)
		ret = pt_task_create(task,name,stksize,prio, NRT);
	else
		ret = pt_task_create(task,name,stksize,prio, RT);
#endif
	if (ret != 0) {
		snprintf(str, sizeof(str), "[ERROR] Failed to create task \"%s\",%d", name, ret);
		perror(str);
	}
	return ret;
}
/*****************************************************************************/
int _set_rt_task_period(RT_TASK *task, SRTIME period) {
	int ret = -1;
	char str[1024]={0,};

#ifdef _XENOMAI_TASKS_
	RT_TASK_INFO info;
	ret = rt_task_inquire(task, &info);
	if (ret != 0) {
		return ret;
	}
	ret = rt_task_set_periodic(task, TM_NOW, period);

#else
	ret = pt_task_set_periodic(task, TM_NOW, period);
	RT_TASK info = *task;
#endif

	if ( ret != 0 ) {
		snprintf(str, sizeof(str), "[ERROR] Failed to make periodic task \"%s\",%d", info.name, ret);
		perror(str);
	}

	return ret;
}
/*****************************************************************************/
int create_rt_task(RT_TASK *task, char *name, int prio) {
	return _create_rt_task(task, name, DEFAULT_TASK_STKSIZE, prio);
}
/*****************************************************************************/
int create_nrt_task(RT_TASK *task, char *name) {
	return _create_rt_task(task, name, DEFAULT_TASK_STKSIZE, 0);
}
/*****************************************************************************/
int set_rt_task_period(RT_TASK *task, RTIME period) {
	return _set_rt_task_period(task, (period));
}
/*****************************************************************************/
void wait_rt_period(RT_TASK *task)
{
#ifdef _XENOMAI_TASKS_
	int ret = -1;
	char str[1024]={0,};
	unsigned long overruns_cnt;
	ret = rt_task_wait_period(&overruns_cnt);
	if (ret != 0) {
			snprintf(str, sizeof(str), "[ERROR] Failed to wait period %d with overruns %lu", ret, overruns_cnt);
			perror(str);
		}

#else
	pt_task_wait_period(task);

	#endif
}
/*****************************************************************************/
int start_rt_task(int enable, RT_TASK *task, void (*fun)(void *cookie)) {
	int ret = -1;
	char str[1024]={0,};

#ifdef _XENOMAI_TASKS_
	RT_TASK_INFO info;
	ret = rt_task_inquire(task, &info);
	if (ret != 0) {
		return ret;
	}
#else
	RT_TASK info = *task;
#endif

	if (enable) {
#ifdef _XENOMAI_TASKS_
		ret = rt_task_start(task, fun, NULL);
#else
		ret = pt_task_start(task, fun, NULL);
#endif
		if (ret != 0) {
			snprintf(str, sizeof(str), "[ERROR] Failed to start RT task \"%s\",%d", info.name, ret);
			perror(str);
		}
		/* code */
	} else {
		fprintf(stderr, "RTtask(%s) is not enabled.\n", info.name);
	}

	return ret;
}
/****************************************************************************/
void delete_rt_task(void)
{
#ifdef _XENOMAI_TASKS_
	int ret = -1;
	char str[1024]={0,};
	ret = rt_task_suspend(NULL);
	if (ret != 0) {
		snprintf(str, sizeof(str), "[ERROR] Failed to suspend rt task %d", ret);
		perror(str);
	}
	ret = rt_task_delete(NULL);
	if (ret != 0) {
		snprintf(str, sizeof(str), "[ERROR] Failed to delete rt task %d", ret);
		perror(str);
	}
#else
		pt_task_delete();
	#endif
}
/****************************************************************************/
