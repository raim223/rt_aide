/*
 *  This file is owned by the Embedded Systems Laboratory of Seoul National University of Science and Technology
 *  to test EtherCAT master protocol using the IgH EtherCAT master userspace library.	
 *  
 *
 *  2020 Raimarius Delgado
*/
/****************************************************************************/
#include <rt_posix_task.h>
/****************************************************************************/
pthread_key_t ptkey_task;
/*****************************************************************************/
const char* _mode_name(PT_MODE mode)
{
	switch(mode)
	{
		case RT:
			return "RT";
			break;
		case NRT:
			return "NRT";
			break;
		default:
			return "Unknown Mode";
			break;
	}
}
/*****************************************************************************/
/* Task status */
/*****************************************************************************/
// static uint32_t _pt_get_status(PT_TASK *task)
// {
// 	return task->status;
// }
/*****************************************************************************/
static uint32_t _pt_test_status(PT_TASK *task, uint32_t bits)
{
	return task->status & bits;
}
/*****************************************************************************/
static void _pt_set_status(PT_TASK *task, uint32_t bits)
{
	task->status |= bits;
}
/*****************************************************************************/
static void _pt_clear_status(PT_TASK *task, uint32_t bits)
{
	task->status &= ~bits;
}
/*****************************************************************************/
int _pt_is_self(PT_TASK *task)
{
	pthread_t t2 = pthread_self();
	return pthread_equal(task->thread, t2);
}
/*****************************************************************************/
/* Task management*/
/*****************************************************************************/
__attribute__ ((constructor)) 
static void  __init_pthreadkey(void) // initiates the task management pthread_key
{
	if (pthread_key_create(&ptkey_task, NULL))
	{
		printf("Cannot initiate pthread_key\n");
		exit(1);
	}
}
/*****************************************************************************/
static void __set_current_task(PT_TASK* task)
{
	pthread_setspecific(ptkey_task, task);
}
/*****************************************************************************/
static PT_TASK *__get_current_task(void)
{
	return (PT_TASK *)pthread_getspecific(ptkey_task);
}
/*****************************************************************************/
static PT_TASK *_get_pt_task_or_self(PT_TASK* task)
{
	if (task)
		return task;

	PT_TASK *current;
	current = __get_current_task();
	if (current == NULL)
	{
		printf("cannot get current_thread\n");
		return NULL;
	}

	return current;
}
/*****************************************************************************/
/* convertion  */
/*****************************************************************************/
struct timespec NS2TIMESPEC(uint64_t nanosecs) // converts nanoseconds to timespec
{
	struct timespec ret; 
	ret.tv_sec = nanosecs / NANOSEC_PER_SEC;
	ret.tv_nsec = nanosecs % NANOSEC_PER_SEC;

	return ret;
}
/*****************************************************************************/
/* task operations */
/*****************************************************************************/
PT_TASK __init_task(void) // initial state of task
{
	PT_TASK task; 
	task.period = 0;
	// task.name = {0,};
	task.taskfcn = NULL;
	task.arg = NULL;
	_pt_set_status(&task, PT_DEAD);
	/* default cpu affinity is CPU 0 */
	CPU_ZERO(&task.affinity);
	CPU_SET(0, &task.affinity);

	return task;
}
/*****************************************************************************/
void *trampoline_task(void *arg)
{
	PT_TASK *my_task = (PT_TASK*) arg;

	my_task->pid = gettid();
	__set_current_task(my_task);

	if (_pt_test_status(my_task,PT_STANDBY|PT_MAPPED) == 0)
		exit(1);

	_pt_clear_status(my_task, PT_TASK_STATES);
	_pt_set_status(my_task, PT_RUN);

	my_task->taskfcn(my_task->arg);

	return NULL;
}
/*****************************************************************************/
int pt_task_create(PT_TASK* task, const char* name, int stksize, int prio, PT_MODE mode)
{
	char str[1024]={0,};
	*task = __init_task(); //initiate a task

	strcpy(task->name, name);
	task->prio = prio;
	task->mode = mode;
	task->s_mode = _mode_name(task->mode);  

	/* initiate pthread attribute */
	int err = pthread_attr_init(&task->thread_attributes);
	if (err)
	{
		snprintf(str, sizeof(str), "[ERROR] attr_init() failed! \"%s\",%d", name, err);
		perror(str);
		return err;
	}

	err = pthread_attr_setdetachstate(&task->thread_attributes, PTHREAD_CREATE_DETACHED /*PTHREAD_CREATE_JOINABLE*/);
	if (err)
	{
		snprintf(str, sizeof(str), "[ERROR] setdetachstate() failed! \"%s\",%d", name, err);
		perror(str);
		return err;
	}

	/* in accordance to RT_PREEMPT document */
	if(mode == RT)
	{
		err = pthread_attr_setinheritsched(&task->thread_attributes, PTHREAD_EXPLICIT_SCHED);
		if (err)
		{
			snprintf(str, sizeof(str), "[ERROR] setinheritedsched() failed! \"%s\",%d", name, err);
			perror(str);
			return err;
		}

		err = pthread_attr_setschedpolicy(&task->thread_attributes, SCHED_FIFO);
		if (err)
		{
			snprintf(str, sizeof(str), "[ERROR] setschedpolicy() failed! \"%s\",%d", name, err);
			perror(str);
			return err;
		}
		struct sched_param paramA = { .sched_priority = prio};
		err = pthread_attr_setschedparam(&task->thread_attributes, &paramA);
		if (err)
		{
			snprintf(str, sizeof(str), "[ERROR] setschedparam() failed! \"%s\",%d", name, err);
			perror(str);
			return err;
		}
		task->prio = prio;
	}
	else
		_pt_set_status(task, PT_NRTTASK);

	/* check if CPU 0 is set */
	if (!(CPU_ISSET(0, &task->affinity)))
	{
		CPU_ZERO(&task->affinity);
		CPU_SET(0, &task->affinity);			
	}
	err = pthread_attr_setaffinity_np(&task->thread_attributes, sizeof(cpu_set_t), &task->affinity);
	if (err)
	{
		snprintf(str, sizeof(str), "[ERROR] setaffinity() failed! \"%s\",%d", name, err);
		perror(str);
		return err;
	}

	/* stksize sanity check */
	if (stksize == 0)
		stksize = 256;
	else if (stksize < 0)
		stksize = -stksize;
	err = pthread_attr_setstacksize(&task->thread_attributes, stksize*1024);
	if (err)
	{
		snprintf(str, sizeof(str), "[ERROR] setstacksize() failed! \"%s\",%d", name, err);
		perror(str);
		return err;
	}

	task->stksize = stksize;
	_pt_set_status(task, PT_STANDBY);


	return 0;
}
/*****************************************************************************/
int pt_task_set_periodic(PT_TASK* task,PRTIME idate, PRTIME period)
{
	int err;
	char str[1024]={0,};
	/* calc start time of the periodic thread */
	struct timespec start_time;
	PT_TASK *task_ptr;

	task_ptr = _get_pt_task_or_self(task);
	if (task_ptr == NULL)
		goto invalid;

	if (_pt_test_status(task_ptr,PT_STANDBY|PT_MAPPED) == 0)
		goto invalid;  

	if (idate == (PRTIME)TMR_NOW){
		if (clock_gettime(CLOCK_TO_USE, &start_time))
		{
			snprintf(str, sizeof(str), "[ERROR] gettime() failed! \"%s\",%d", task_ptr->name, err);
			perror(str);
			return err;
		}
	} else
		/*
		 * idate is an absolute time specification
		 * already, so we want a direct conversion to
		 * timespec.
		 */
		start_time = NS2TIMESPEC(idate);

	/* add period to start_time */ 
	start_time.tv_nsec += period;
	start_time.tv_sec += start_time.tv_nsec / NANOSEC_PER_SEC;
	start_time.tv_nsec %= NANOSEC_PER_SEC;

	task_ptr->deadline = start_time;
	task_ptr->period = period;

	_pt_set_status(task_ptr, PT_PERIODIC);

	return 0;

invalid:
	printf("cannot set_periodic:invalid task parameters!\n");
	return -EWOULDBLOCK;
}
/*****************************************************************************/
int pt_task_start(PT_TASK* task,void (*entry)(void *arg), void* arg)
{
	int err;
	char str[1024]={0,};

	if (_pt_test_status(task,PT_STANDBY) == 0)
		return -EWOULDBLOCK;

	task->taskfcn = entry;
	task->arg = arg;

	err = pthread_create(&task->thread, &task->thread_attributes, trampoline_task, task);
	if (err)
	{
		snprintf(str, sizeof(str), "[ERROR] pthread_create() failed! \"%s\",%d", task->name, err);
		perror(str);
		return err;
	}
	else
	{
		_pt_set_status(task,PT_MAPPED);
		__set_current_task(task);

		pthread_attr_destroy(&task->thread_attributes);
		err = pthread_setname_np(task->thread, task->name);
		if (err)
		{
			snprintf(str, sizeof(str), "[ERROR] setname() failed! \"%s\",%d", task->name, err);
			perror(str);
			return err;
		}
	}
		return 0;
}
/*****************************************************************************/
int pt_task_wait_period(unsigned long *overruns_cnt)
{
	PT_TASK *task;
	task = __get_current_task();
	if (task == NULL)
		return -EWOULDBLOCK;
		
	if (_pt_test_status(task, PT_PERIODIC))
	{
		int err = 0;
		struct timespec now;

		_pt_clear_status(task, PT_TASK_STATES);
		_pt_set_status(task, PT_READY);
		err =clock_nanosleep(CLOCK_TO_USE,TIMER_ABSTIME,&task->deadline,NULL);
		if ( err>0 )
		{
			printf("Timer wait period failed! \"%s\",%d", task->name, err);
		}
		_pt_clear_status(task, PT_TASK_STATES);
		_pt_set_status(task, PT_RUN);
		
		task->deadline.tv_nsec += task->period;
		task->deadline.tv_sec += task->deadline.tv_nsec / NANOSEC_PER_SEC;
		task->deadline.tv_nsec %= NANOSEC_PER_SEC;
		now =  NS2TIMESPEC(pt_timer_read());

		/* check for missed deadlines */
		if (now.tv_sec > task->deadline.tv_sec || (now.tv_sec == task->deadline.tv_sec && task->deadline.tv_nsec > now.tv_nsec))
	 	{
	 		*overruns_cnt += 1;
	 	}
	 	return 0;
	}
	else
		return -EWOULDBLOCK;
}
/*****************************************************************************/
PRTIME pt_timer_read(void){
	struct timespec probe;
	// PRTIME ret;
	if (clock_gettime(CLOCK_TO_USE,&probe))
		{
			printf("Failed to clock_gettime probe\n" );
			return -1;
		}
	else
	{
		return (TIMESPEC2NS(probe));
	}
}
/*****************************************************************************/
void pt_timer_spin(PRTIME spintime)
{
	PRTIME end;
	end = pt_timer_read() + spintime;
	while (pt_timer_read() < end)
		cpu_relax();
}
/*****************************************************************************/
PRTIME pt_timer_ns2ticks(PRTIME ticks)
{
	return ticks;
}
/*****************************************************************************/
int pt_task_inquire(PT_TASK* task, PT_TASK_INFO* info)
{
	PT_TASK *task_ptr;

	task_ptr = _get_pt_task_or_self(task);
	if (task_ptr == NULL)
		return -EPERM;

	info->status = task_ptr->status;
	strcpy(info->name, task_ptr->name);
	info->priority = task_ptr->prio;
	info->pid = task_ptr->pid; 

	return 0;
}
/*****************************************************************************/
int pt_task_suspend(PT_TASK* task)
{
	PT_TASK *task_ptr;
	pid_t pid;
	int ret; 

	task_ptr = _get_pt_task_or_self(task);
	if (task_ptr == NULL)
		return -EPERM;

	pid = task_ptr->pid;

	if (task_ptr->status & (PT_SUSP|PT_DEAD))
		return 0;		

	ret = kill(pid,SIGRTMAX + 1);

	_pt_clear_status(task, PT_TASK_STATES);
	_pt_set_status(task, PT_SUSP);

	return ret;
}
/*****************************************************************************/
int pt_task_delete(PT_TASK* task)
{
	PT_TASK *task_ptr;

	task_ptr = _get_pt_task_or_self(task);
	if (task_ptr == NULL)
		return -EPERM;

	if (task_ptr->status & PT_DEAD)
		return 0;	

	if (task_ptr == __get_current_task())
	{
		pthread_exit(NULL);
	}

	_pt_clear_status(task, PT_TASK_STATES);
	_pt_set_status(task, PT_DEAD);

	return 0;
}
/*****************************************************************************/

