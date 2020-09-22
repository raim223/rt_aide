#ifndef _RT_POSIX_TASK_H_
#define _RT_POSIX_TASK_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE //for pthread_setname_np
#endif

/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <sys/syscall.h>
#include <string.h>
#include <signal.h>
/*****************************************************************************/
#include <embdCOMMON.h>
/*****************************************************************************/
#define START_DELAY_SECS 0 //1sec
#define NANOSEC_PER_SEC 1000000000
#define CLOCK_TO_USE CLOCK_MONOTONIC
#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NANOSEC_PER_SEC + (T).tv_nsec)
#define TMR_NOW (-99)
#define PREDEFINED_STKSIZE (32) //for 32 kb
#define AIDE_MAX_NAME_LEN 32


/* real-time task state-machine */
#define PT_STANDBY 		0x00000001 // when task is created
#define PT_READY 		0x00000002 // task is scheduled but not yet running
#define PT_DEAD 		0x00000004 // task is deleted or not created
#define PT_RUN 			0x00000008 // running task
#define PT_SUSP			0x00000010 // running task
#define PT_TASK_STATES 	(PT_STANDBY|PT_READY|PT_DEAD|PT_RUN|PT_SUSP)

/* Signals */
#define PTSIGSUSP		(SIGRTMAX + 1)
#define PTSIGRESM		(SIGRTMAX + 2)

#define PT_MAPPED		0x00000100
#define PT_PERIODIC 	0x00001000 // periodic task
#define PT_NRTTASK	 	0x10000000 // periodic task

#define gettid()		(pid_t)(syscall(SYS_gettid))

/* source-specific definitions */
typedef uint64_t PRTIME; //for timer probe
typedef TASK_TYPE PT_MODE;

/*****************************************************************************/
typedef struct{
	/* task thread and thread attributes handler */
	pthread_t thread;
	pthread_attr_t thread_attributes;
	pid_t pid; // process ID
	/* task specifications */
	uint32_t status;
	int prio;
	int stksize;
	PT_MODE mode;
	cpu_set_t affinity;
	const char* s_mode;
	char name[AIDE_MAX_NAME_LEN];
	PRTIME period;
	/* timer related (for periodic tasks) */
	struct timespec deadline;
	/* task loop and arguments */
	void (*taskfcn)(void *arg);
	void *arg;
}PT_TASK;
/*****************************************************************************/
typedef struct{
	int priority;
	uint32_t status;
	char name[AIDE_MAX_NAME_LEN];
	pid_t pid;
}PT_TASK_INFO;
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************/
/* Creation of real-time periodic task using POSIX  
 * PT_TASK -> task descriptor
 * name -> Desired name of task 
 * stksize ->  the size of stack the system will allocate for the task (0 = 256 kb) 
 * prio -> 0~99 with 99 as the highest priority
 * mode -> specify wheter the task is RT (real-time) or NRT (non real-time)
 *****************************************************************************/
int pt_task_create(PT_TASK* task,const char* name, int stksize, int prio, PT_MODE mode);
/*****************************************************************************/
/* makes the task periodic */
/*****************************************************************************/ 
int pt_task_set_periodic(PT_TASK* task,PRTIME idate, PRTIME period);
/*****************************************************************************/
/* starts the task
 *****************************************************************************/
int pt_task_start(PT_TASK* task,void (*entry)(void *arg) , void* arg);
/*****************************************************************************/
/* waits for the next activation point using clock_nanosleep */
/*****************************************************************************/
int pt_task_wait_period(unsigned long *overruns_cnt);
/*****************************************************************************/
/* Returns the current system time expressed in nanoseconds */
/*****************************************************************************/ 
PRTIME pt_timer_read(void);
/*****************************************************************************/
/* Spins the CPU doing nothing for "spintime" nanoseconds */
/*****************************************************************************/
void pt_timer_spin(PRTIME spintime);
/*****************************************************************************/
PRTIME pt_timer_ns2ticks(PRTIME ticks);
/*****************************************************************************/
int pt_task_inquire(PT_TASK* task, PT_TASK_INFO* info);
/*****************************************************************************/
int pt_task_suspend(PT_TASK* task);
/*****************************************************************************/
int pt_task_delete(PT_TASK* task);
/*****************************************************************************/

#ifdef __cplusplus
}
#endif 
#endif //_RT_POSIX_TASK_H_