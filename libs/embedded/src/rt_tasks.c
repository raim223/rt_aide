#include <rt_tasks.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/*****************************************************************************/
#define DEFAULT_TASK_STKSIZE 0
// int _create_rt_task(RT_TASK *task, const char *name, int stksize, int prio);
// int _set_rt_task_period(RT_TASK *task, SRTIME period);
// int _start_rt_task(int enable, RT_TASK *task, void (*entry)(void *arg), void *arg);
/*****************************************************************************/
int _create_rt_task(RT_TASK *task, const char *name, int stksize, int prio) {
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
	PT_TASK_INFO info;
	ret = pt_task_inquire(task, &info);
	if (ret != 0) {
		return ret;
	}
	ret = pt_task_set_periodic(task, TM_NOW, period);
#endif

	if ( ret != 0 ) {
		snprintf(str, sizeof(str), "[ERROR] Failed to make periodic task \"%s\",%d", info.name, ret);
		perror(str);
	}

	return ret;
}
/*****************************************************************************/
int _start_rt_task(int enable, RT_TASK *task, void (*entry)(void *arg), void *arg)
{
	int ret = -1;
	char str[1024]={0,};

	if (enable)
	{
#ifdef _XENOMAI_TASKS_
		RT_TASK_INFO info;
		ret = rt_task_inquire(task, &info);
		if (ret != 0) {
			return ret;
		}
		ret = rt_task_start(task, entry, arg);

#else
		PT_TASK_INFO info;
		ret = pt_task_inquire(task, &info);
		if (ret != 0) {
			return ret;
		}
		ret = pt_task_start(task, entry, arg);
#endif
		if (ret != 0) {
			snprintf(str, sizeof(str), "[ERROR] Failed to start RT task \"%s\",%d", info.name, ret);
			perror(str);
		}
	}
	else {
		// fprintf(stderr, "RTtask(%s) is not enabled.\n", info.name);
		fprintf(stderr, "RTtask is not enabled.\n");
	}
	return ret;
}
/*****************************************************************************/
/* Public Functions */
/*****************************************************************************/
int aide_create_rt_task(RT_TASK *task, const char *name, int prio) {
	return _create_rt_task(task, name, DEFAULT_TASK_STKSIZE, prio);
}
/*****************************************************************************/
int aide_create_nrt_task(RT_TASK *task, const char *name) {
	return _create_rt_task(task, name, DEFAULT_TASK_STKSIZE, 0);
}
/*****************************************************************************/
int aide_set_task_period(RT_TASK *task, RTIME period) {
	return _set_rt_task_period(task, (period));
}
/*****************************************************************************/
int aide_start_task(int enable, RT_TASK *task, void (*entry)(void *arg), void *arg)
{
	return _start_rt_task(enable,task,entry, arg);
}
/*****************************************************************************/
int aide_start_task_noarg(int enable, RT_TASK *task, void (*entry)(void *arg))
{
	return _start_rt_task(enable,task,entry, NULL);
}
/*****************************************************************************/
int aide_wait_period(unsigned long *overruns_cnt)
{
	int ret = -1;
	char str[1024]={0,};
#ifdef _XENOMAI_TASKS_
	ret = rt_task_wait_period(overruns_cnt);
#else
	ret = pt_task_wait_period(overruns_cnt);
#endif
	if (ret != 0) {
			snprintf(str, sizeof(str), "[ERROR] Failed to wait period %d with overruns %lu", ret, *overruns_cnt);
			perror(str);
		}
	return	ret;
}
/*****************************************************************************/
int aide_suspend_task(RT_TASK *task)
{
	int ret = -1;
	char str[1024]={0,};

#ifdef _XENOMAI_TASKS_
	RT_TASK_INFO info;
	ret = rt_task_inquire(task, &info);
	if (ret != 0) {
		return ret;
	}
	ret = rt_task_suspend(task);
#else
	PT_TASK_INFO info;
	ret = pt_task_inquire(task, &info);
	if (ret != 0) {
		return ret;
	}
	ret = pt_task_suspend(task);
#endif

	if (ret != 0) {
		snprintf(str, sizeof(str), "[ERROR] Failed to suspend task \"%s\",%d", info.name, ret);
		perror(str);
	}
	return ret;
}
/*****************************************************************************/
int aide_delete_task(RT_TASK *task)
{
	int ret = -1;
	char str[1024]={0,};

#ifdef _XENOMAI_TASKS_
	RT_TASK_INFO info;
	ret = rt_task_inquire(task, &info);
	if (ret != 0) {
		return ret;
	}
	ret = rt_task_delete(task);
#else
	PT_TASK_INFO info;
	ret = pt_task_inquire(task, &info);
	if (ret != 0) {
		return ret;
	}
	ret = pt_task_delete(task);
#endif

	if (ret != 0) {
		snprintf(str, sizeof(str), "[ERROR] Failed to delete task \"%s\",%d", info.name, ret);
		perror(str);
	}
	return ret;
}
/****************************************************************************/
