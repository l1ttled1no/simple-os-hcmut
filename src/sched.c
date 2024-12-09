
#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include <timer.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int flag[MAX_PRIO];

#endif

int queue_empty(void)
{
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if (!empty(&mlq_ready_queue[prio]))
			return -1;
	return 1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void)
{
#ifdef MLQ_SCHED
	for (int i = 0; i < MAX_PRIO; i++)
	{
		mlq_ready_queue[i].size = 0;
		mlq_ready_queue[i].current_time = 0;
		flag[i] = 0;
	}

#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/*
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
void reset_queue()
{
	for (int i = 0; i < MAX_PRIO; i++)
	{
		mlq_ready_queue[i].current_time = 0;
		flag[i] = 0;
	}
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}
struct pcb_t *get_mlq_proc(void) {
    struct pcb_t *proc = NULL;
    
    
    int current_cycle = -1;
	int max_cycle = -1;
	
	//Set cycle for each process in ready queue
	for (int prio = 0; prio < MAX_PRIO; prio++) {
        if (mlq_ready_queue[prio].size > 0) {
            if (current_cycle == -1) current_cycle = flag[prio];
            current_cycle = min(current_cycle, flag[prio]);
            max_cycle = max(max_cycle, flag[prio]);
        }
    }
	pthread_mutex_lock(&queue_lock);
    // First attempt: try to get process from current cycle
    for (int prio = 0; prio < MAX_PRIO; prio++) {
        if (mlq_ready_queue[prio].size > 0 && mlq_ready_queue[prio].current_time < MAX_PRIO - prio && flag[prio] == current_cycle) {
            proc = dequeue(&mlq_ready_queue[prio]);
            mlq_ready_queue[prio].current_time++;
            pthread_mutex_unlock(&queue_lock);
            return proc;
        }
    }
	// 1. All queues are empty in current cycle (can existing some Process are dispatching)
	// 2. All queues in current cycle have used up their time slots.

	// Second attempt: try to get process from next cycle (max - current cycle <= 1)
	for (int prio = 0; prio < MAX_PRIO; prio++) {
        if (mlq_ready_queue[prio].size > 0 && mlq_ready_queue[prio].current_time < MAX_PRIO - prio && flag[prio] == max_cycle) {
            proc = dequeue(&mlq_ready_queue[prio]);
            mlq_ready_queue[prio].current_time++;
            pthread_mutex_unlock(&queue_lock);
            return proc;
        }
    }

    // Process still NULL
	// Set new cycle for highest priority
    for (int prio = 0; prio < MAX_PRIO; prio++) {
        if (mlq_ready_queue[prio].size > 0 && flag[prio] == current_cycle){
			flag[prio]++;
			mlq_ready_queue[prio].current_time = 0;
            proc = dequeue(&mlq_ready_queue[prio]);
            mlq_ready_queue[prio].current_time++;
			pthread_mutex_unlock(&queue_lock);
            return proc;
        }
    }
    pthread_mutex_unlock(&queue_lock);
    return proc;
}

void put_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	//Check condition of proc prio
	if (proc->prio < 0 || proc->prio >= MAX_PRIO){
		pthread_mutex_unlock(&queue_lock);
		return;
	}
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	if (proc->prio < 0 || proc->prio >= MAX_PRIO){
		pthread_mutex_unlock(&queue_lock);
		return;
	}	
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

struct pcb_t *get_proc(void)
{
	return get_mlq_proc();
}

void put_proc(struct pcb_t *proc)
{
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t *proc)
{
	//printQueue(&mlq_ready_queue[0]);
	return add_mlq_proc(proc);
}


#else
struct pcb_t *get_proc(void)
{
	struct pcb_t *proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	// Copy element from ready_queue to run_queue
	pthread_mutex_lock(&queue_lock);
	if (ready_queue.size == 0 && run_queue.size > 0)
	{
		ready_queue.size = run_queue.size;
		for (int i = 0; i < ready_queue.size; i++)
		{
			ready_queue.proc[i] = run_queue.proc[i];
			run_queue.proc[i] = NULL;
		}
		run_queue.size = 0;
		proc = dequeue(&ready_queue);
	}
	// Get the first element from the ready_queue since the queue is sorted
	else if (ready_queue.size > 0)
	{
		proc = dequeue(&ready_queue);
	}
	pthread_mutex_unlock(&queue_lock);

	return proc;
}

void put_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}
#endif