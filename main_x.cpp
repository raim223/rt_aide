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

#define TASK_1_PRIO		(99) // highest priority  
#define TASK_1_PRD		(1000000000) // 1 second period
const char *sTaskName1 = "task_1";
FLAG bQuitFlag = off;
/*****************************************************************************/
/* function macros */
/*****************************************************************************/
void XenoInit();
void XenoStart();
void SignalHandler(int signum);
/****************************************************************************/
void TestTask1(void *arg){
	RTIME rtmPrdCurr=0, rtmPrdPrev=0; 
	int tmPrd=0;

	RT_TASK_INFO info;
	rt_task_inquire(NULL, &info);
	
	rtmPrdPrev = rt_timer_read();
	while (1) {
		if (bQuitFlag == ON){
			delete_rt_task();
			break;
		}else
			wait_rt_period(&TskTest1);

		RT_TASK_INFO info;
		rt_task_inquire(NULL, &info);


		rtmPrdCurr = rt_timer_read();
		tmPrd = (int)(rtmPrdCurr - rtmPrdPrev);
		rtmPrdPrev = rtmPrdCurr;
		printf("0x%x\n",info.stat.status);
		printf("%d\n",info.pid);
		// printf("Task Period: %d.%06d\n", tmPrd/1000000000,tmPrd%1000000000);
	}
}
/****************************************************************************/
int main(int argc, char **argv){

	/* Interrupt Handler "ctrl+c"  */
	signal(SIGTERM, SignalHandler);
	signal(SIGINT, SignalHandler);

	// void *(*fun)(void *cookie);

	/* RT-tasks */
	mlockall(MCL_CURRENT|MCL_FUTURE); 
	XenoInit();
	XenoStart();

	while (1) {
		usleep(1);
		if (bQuitFlag==ON) break;
	}
	return 0;
}
/****************************************************************************/
void SignalHandler(int signum){
		bQuitFlag=ON;
}
/****************************************************************************/
void XenoInit(){
	printf("Creating Real-time task(s)...");
	create_rt_task(&TskTest1,sTaskName1, TASK_1_PRIO);
	printf("OK!\n");

	printf("Making Real-time task(s) Periodic...");
	set_rt_task_period(&TskTest1,TASK_1_PRD);
	printf("OK!\n");
}
/****************************************************************************/
void XenoStart(){
	printf("Starting Real-time Task(s)...");
	start_rt_task_noarg(1,&TskTest1,TestTask1);
	printf("OK!\n");
}
/***************************************************************************/