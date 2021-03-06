#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sched.h>
#include <pthread.h>
#include <native/task.h>
#include <native/timer.h>
#include <rtdk.h>
#include <sys/io.h>
#include "io.h"


RT_TASK tasks[3];

int set_cpu(int cpu_number)
{
	// setting cpu set to the selected cpu
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);
	// set cpu set to current thread and return
	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}

void catch_signal(int sig) {}

void wait_for_ctrl_c() {
	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);
	// wait for SIGINT (CTRL-C) or SIGTERM signal
	pause();
	printf(" Snakkes.\n");
}

/*
void* periodicTest(void *data) {
  	int pin = (int *)data;
  	printf("Thread started pin %d\n", pin);
	rt_task_set_periodic(NULL, TM_NOW, 1);
  	while(1) {
		while(io_read(pin)) {
			rt_task_wait_period(NULL);
		}
    		int tmp = io_read(pin);
    		io_write(pin, tmp);
  	}
  	pthread_exit(NULL);
}*/

void* periodicTest(void *data) {
	int pin = (int*) data;
	set_cpu(1);
	io_write(pin+1,1);
	rt_task_set_periodic(&tasks[pin], TM_NOW, 50000);
	while(1) {
		//printf("PIN: %d\n", pin);
		if(io_read(pin+1) == 0) {
			io_write(pin+1,0);
			usleep(5);
			io_write(pin+1,1);
			//int tmp = io_read(pin);
    			//io_write(pin, tmp);
		}
		rt_task_wait_period(NULL);
	}
}
	
void periodicPrint()
{
	rt_task_set_periodic(NULL, TM_NOW, 1000000000);
	while(1)
	{
		printf("Maja-hi, Maja-ho, Maja-ha, Maja-ha-ha\n");
		rt_task_wait_period(NULL);
	}
}


void* disturbance() {
	set_cpu(1);
	double i, j, k;
	while(1) {
		i++;
		j++;
		k = i*j;
		k++;
	}
}


int main() {
	if (io_init() != 1) {
		printf("io_init failed\n");
	}
	
	//periodicPrint();
	
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_print_auto_init(1);

	long i;
	for (i=0; i<3; i++) {
		rt_task_create(&tasks[i], "test"+i, 0, i+1, T_JOINABLE);
	} 

	for (i=0; i<3; i++) {
		rt_task_start(&tasks[i], &periodicTest, (void*) i);
	}

	for (i=0; i<3; i++) {
		rt_task_join(&tasks[i]);
	}
	
	pthread_t disturbances[10];

	for (i=0; i <10; i++) {
		pthread_create(&disturbances[i], NULL, disturbance, NULL);
	}

	for(i=0; i<10; i++) {
		pthread_join(disturbances[i], NULL);
	}

	wait_for_ctrl_c();

	return 0;
}
