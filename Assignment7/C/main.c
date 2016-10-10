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

static RT_MUTEX aMutex, bMutex;

RT_TASK L, H; 

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
	rt_sem_p(&sem, TM_INFINITE);
	rt_mutex_acquire(&aMutex,TM_INFINITE);
	rt_task_set_priority(&L, 20);
	rt_printf("Low locks A\n");
	busy_wait_ms(300);
	rt_mutex_acquire(&bMutex, TM_INFINITE);
	rt_printf("Low locks B\n");
	busy_wait_ms(300);
	rt_mutex_release(&bMutex);
	rt_printf("Low unlocks B\n");
	rt_mutex_release(&aMutex);
	rt_printf("Low unlocks A\n");
	rt_task_set_priority(&L, 5);	
	busy_wait_ms(100);
}


void waitHigh() {
	rt_sem_p(&sem, TM_INFINITE);
	rt_task_sleep_ms(100);
	rt_mutex_acquire(&bMutex, TM_INFINITE);
	rt_printf("High locks B\n");
	busy_wait_ms(100);
	rt_mutex_acquire(&aMutex, TM_INFINITE);
	rt_printf("High locks A\n");
	busy_wait_ms(200);
	rt_mutex_release(&aMutex);
	rt_printf("High unlocks A\n");
	rt_mutex_release(&bMutex);
	rt_printf("High unlocks B\n");
	busy_wait_ms(100);
}

void syncFunc() {
	usleep(100000);
	rt_sem_broadcast(&sem);
	usleep(100000);
}

int main() {
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_print_auto_init(1);
	
	RT_TASK sync;
	
	rt_task_create(&L, "l", 0, 5, T_CPU(1)|T_JOINABLE);
	rt_task_create(&H, "h", 0, 10, T_CPU(1)|T_JOINABLE);
	rt_task_create(&sync, "sync", 0, 99, T_CPU(1)|T_JOINABLE);

	rt_sem_create(&sem, "sem", 0, S_PRIO);
	rt_mutex_create(&aMutex, "aMutex");
	rt_mutex_create(&bMutex, "bMutex");

	rt_task_start(&L, &waitLow, NULL);
	rt_task_start(&H, &waitHigh, NULL);
	rt_task_start(&sync, &syncFunc, NULL);

	rt_task_join(&L);
	rt_task_join(&H);
	rt_task_join(&sync);

	rt_sem_delete(&sem);
	
	rt_mutex_delete(&aMutex);
	rt_mutex_delete(&bMutex);
	return 0;
}
