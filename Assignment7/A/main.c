#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sched.h>
#include <pthread.h>
#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <rtdk.h>
#include <sys/io.h>


RT_SEM sem;

void waitLow() {
	rt_sem_p(&sem, TM_INFINITE);
	rt_printf("Low priority task finished.\n");
}

void waitHigh() {
	rt_sem_p(&sem, TM_INFINITE);
	rt_printf("High priority task finished.\n");
}

void syncFunc() {
	usleep(100000);
	rt_sem_broadcast(&sem);
	usleep(100000);
}

int main() {
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_print_auto_init(1);

	RT_TASK low, high, sync;
	rt_task_create(&low, "low", 0, 0, T_CPU(1)|T_JOINABLE);
	rt_task_create(&high, "high", 0, 1, T_CPU(1)|T_JOINABLE);
	rt_task_create(&sync, "sync", 0, 99, T_CPU(1)|T_JOINABLE);

	rt_sem_create(&sem, "sem", 0, S_PRIO);

	rt_task_start(&low, &waitLow, NULL);
	rt_task_start(&high, &waitHigh, NULL);
	rt_task_start(&sync, &syncFunc, NULL);

	rt_task_join(&low);
	rt_task_join(&high);
	rt_task_join(&sync);

	rt_sem_delete(&sem);

	return 0;
}
