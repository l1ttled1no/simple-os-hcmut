
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
    int i ;

	for (i = 0; i < MAX_PRIO; i ++)
		mlq_ready_queue[i].size = 0;
		mlq_ready_queue[i].time_slot = 0;
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
void reset_queue(){
	for (int i = 0; i < MAX_PRIO; i++){
		mlq_ready_queue[i].time_slot = 0;
	}
}

struct pcb_t * get_mlq_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */

	//What to do
	//Check if the queue is empty or not
	//Loop through each priority queue
	//If a queue is empty -> go to next queue
	//Compute the slots of this queue
	//If a queue is run out if slot -> go to next queue
	//If all queue run out of slots -> reset
	if (queue_empty()) return NULL;
	struct queue_t* current_queue;
	int queue_index = 0;
	int queue_max_slot;

	while (1){
		//Loop through each priority queue
		//Check if the current priority queue is empty or not
		current_queue = &mlq_ready_queue[queue_index];
		if (current_queue->size >0 ){
			//Check if it still has slots or not
			queue_max_slot = MAX_QUEUE_SIZE - queue_index;
			if (current_queue->time_slot >= queue_max_slot){
				queue_index++;
			}
			else{
					//Get the process from the queue
					pthread_mutex_lock(&queue_lock);
					proc = dequeue(current_queue);
					current_queue->time_slot++;
					pthread_mutex_unlock(&queue_lock);
					return proc;
			}
		}
		else {
			queue_index++;
		}
		//All the queue are run out of slots and still not empty -> reset
		if (queue_index >= MAX_QUEUE_SIZE-1){
			queue_index = 0;
			reset_queue();
		}
	}


	return proc;	
}

void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t * proc) {
	return add_mlq_proc(proc);
}
#else
struct pcb_t * get_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	//Copy element from ready_queue to run_queue
	pthread_mutex_lock(&queue_lock);
	if(ready_queue.size==0 && run_queue.size >0)
	{
		ready_queue.size=run_queue.size;
		for(int i=0;i<ready_queue.size ;i++)
		{
			ready_queue.proc[i]=run_queue.proc[i];
			run_queue.proc[i]=NULL;	
		}
		run_queue.size=0;
		proc=dequeue(&ready_queue);
	}
	//Get the first element from the ready_queue since the queue is sorted
	else if(ready_queue.size >0)
	{
		proc=dequeue(&ready_queue);
	}
	pthread_mutex_unlock(&queue_lock);

	return proc;
}

void put_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);	
}
#endif


