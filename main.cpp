/*
 *  This file is owned by the Embedded Systems Laboratory of Seoul National University of Science and Technology
 *  Simple RT-AIDE example
 *  2020 Raimarius Delgado
*/
/****************************************************************************/
#include <embdCOMMON.h>  //./libs/embedded
#include <embdMATH.h>  //./libs/embedded
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <malloc.h>
#include <pthread.h>
#include <ctype.h>
/*****************************************************************************/
/* RT_TASKS */
/*****************************************************************************/
#include <rt_tasks.h>

/* task creation */
RT_TASK TskTest1;
RT_TASK TskTest2;
#define TASK_1_PRIO		(99) // highest priority  
#define TASK_1_PRD		(1000000000) // 1 second period
const char *sTaskName1 = "task_1";
#define TASK_2_PRIO		(89) // highest priority  
#define TASK_2_PRD		(2000000000) // 1 second period
const char *sTaskName2 = "task_2";
FLAG bQuitFlag = off;
/****************************************************************************/
void TestTask1(void *arg){
	RTIME rtmPrdCurr=0, rtmPrdPrev=0; 
	int tmPrd=0;

	int sample = *(int*)arg;
	int iCnt=0; 

	rtmPrdPrev = rt_timer_read();
	while (1) {
		aide_wait_period(NULL);

		// RT_TASK_INFO info;
		// rt_task_inquire(NULL, &info);


		rtmPrdCurr = rt_timer_read();
		tmPrd = (int)(rtmPrdCurr - rtmPrdPrev);
		rtmPrdPrev = rtmPrdCurr;
		// printf("0x%x\n",info.stat.status);
		// printf("%d\n",info.pid);

		printf("Task1: sample: %d %d.%06d\n",sample, tmPrd/1000000000,tmPrd%1000000000);
		// printf("Task1 thead: %lu\n", TskTest1.thread);
		// printf("Task1 self_thread: %lu\n", TskTest1.self_thread);
		// printf("Task1 task_self_thread: %lu\n", self_thread);
		// printf("Task1 PID: %d\n", TskTest1.pid);
		// printf("Task1 Name: %s\n", TskTest1.name);

		if (iCnt++ == 10)
		{
			aide_suspend_task(NULL);
			aide_delete_task(NULL);
		}
	}
}
/****************************************************************************/
void TestTask2(void *arg){
	RTIME rtmPrdCurr=0, rtmPrdPrev=0; 
	int tmPrd=0;
	aide_set_task_period(NULL,TASK_2_PRD);

	int sample = *(int*)arg;

	rtmPrdPrev = rt_timer_read();
	while (1) {
		if (bQuitFlag == ON){
			// delete_rt_task();
			break;
		}else
			aide_wait_period(NULL);

		// RT_TASK_INFO info;
		// rt_task_inquire(NULL, &info);


		rtmPrdCurr = rt_timer_read();
		tmPrd = (int)(rtmPrdCurr - rtmPrdPrev);
		rtmPrdPrev = rtmPrdCurr;
		// printf("0x%x\n",info.stat.status);
		// printf("%d\n",info.pid);
		printf("Task2: sample: %d %d.%06d\n",sample, tmPrd/1000000000,tmPrd%1000000000);
		// printf("Task2 PID: %d\n", TskTest2.pid);
		// printf("Task2 thead: %lu\n", TskTest2.thread);
		// printf("Task2 self_thread: %lu\n", TskTest2.self_thread);
		// printf("Task2 task_self_thread: %lu\n", self_thread);
		// printf("Task2 PID: %d\n", TskTest2.pid);
	}
}

/****************************************************************************/
void SignalHandler(int signum){
		bQuitFlag=ON;
}
/****************************************************************************/
int main(int argc, char **argv){

	int argSample = 5;

	/* Interrupt Handler "ctrl+c"  */
	signal(SIGTERM, SignalHandler);
	signal(SIGINT, SignalHandler);

	/* RT-tasks */
	mlockall(MCL_CURRENT|MCL_FUTURE); 
	
	printf("Creating Real-time task(s)...");
	aide_create_rt_task(&TskTest1,sTaskName1, TASK_1_PRIO);
	aide_create_rt_task(&TskTest2,sTaskName2, TASK_2_PRIO);	
	printf("OK!\n");

	printf("Making Real-time task(s) Periodic...");
	aide_set_task_period(&TskTest1,TASK_1_PRD);
	
	printf("OK!\n");

	// printf("Starting Real-time Task(s)...");
	aide_start_task(1,&TskTest1,&TestTask1,&argSample);
	// argSample += 10;
	// aide_start_task(0,&TskTest2,&TestTask2,&argSample);
	// printf("OK!\n");

	while (1) {
		usleep(1);
		if (bQuitFlag==ON) break;
	}
	return 0;
}
/****************************************************************************/
