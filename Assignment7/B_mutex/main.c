#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sched.h>
#include <pthread.h>
#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <native/mutex.h>
#include <rtdk.h>
#include <sys/io.h>


RT_SEM sem;
RT_SEM rSem;

static RT_MUTEX rMutex;

void print_pri(RT_TASK *task, char *s) {
	struct rt_task_info temp;
	rt_task_inquire(task, &temp);
	rt_printf("b:%i c:%i ", temp.bprio, temp.cprio);
	rt_printf(s);
}

int rt_task_sleep_ms(unsigned long delay) {
	return rt_task_sleep(1000*1000*delay);
}

void busy_wait_ms(unsigned long delay){
	unsigned long count = 0;
	while (count <= delay*10){
		rt_timer_spin(1000*100);
		count++;
	}
}

void waitLow() {
	rt_printf("Low waiting..\n");
	rt_sem_p(&sem, TM_INFINITE);
	rt_mutex_acquire(&rMutex,TM_INFINITE);
	rt_printf("Low locks\n");
	busy_wait_ms(300);
	rt_mutex_release(&rMutex);
	rt_printf("Low unlocks\n");
}

void waitMed() {
	rt_printf("Med waiting..\n");
	rt_sem_p(&sem, TM_INFINITE);
	rt_printf("Medium starts\n");
	rt_task_sleep_ms(100);
	busy_wait_ms(500);
	rt_printf("Medium finished\n");
}
	

void waitHigh() {
	rt_printf("High waiting..\n");
	rt_sem_p(&sem, TM_INFINITE);
	
	rt_task_sleep_ms(200);
	rt_printf("High locks\n");
	rt_mutex_acquire(&rMutex, TM_INFINITE);
	busy_wait_ms(200);
	rt_mutex_release(&rMutex);
	rt_printf("High finished\n");
}

void syncFunc() {
	usleep(100000);
	rt_sem_broadcast(&sem);
	usleep(100000);
}

int main() {
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_print_auto_init(1);
	RT_TASK L, M, H, sync;
	
	rt_task_create(&L, "l", 0, 0, T_CPU(1)|T_JOINABLE);
	rt_task_create(&M, "m", 0, 1, T_CPU(1)|T_JOINABLE);
	rt_task_create(&H, "h", 0, 2, T_CPU(1)|T_JOINABLE);
	rt_task_create(&sync, "sync", 0, 99, T_CPU(1)|T_JOINABLE);

	rt_sem_create(&sem, "sem", 0, S_PRIO);
	rt_sem_create(&rSem, "rSem", 1, S_PRIO);
	rt_mutex_create(&rMutex, "rMutex");

	rt_task_start(&L, &waitLow, NULL);
	rt_task_start(&M, &waitMed, NULL);
	rt_task_start(&H, &waitHigh, NULL);
	rt_task_start(&sync, &syncFunc, NULL);

	rt_task_join(&L);
	rt_task_join(&M);
	rt_task_join(&H);
	rt_task_join(&sync);

	rt_sem_delete(&sem);
	rt_sem_delete(&rSem);
	rt_mutex_delete(&rMutex);

	return 0;
}
