#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
#ifdef MLQ_SCHED
        if (q->size < MAX_QUEUE_SIZE){
                //Put the process to then end of the queue
                q->proc[q->size-1] = proc;
                //Increase the size of the queue
                q->size++;
        }
#else
        //This queue is sorted
        if (q->size < MAX_QUEUE_SIZE){
                //Put the process in the correct position
                int position = q->size - 1;
                while (position > 0){
                        if (q->proc[position]->priority > proc->priorit){
                                q->proc[position+1] = q->proc[position];
                        }
                        position--;
                }
                q->proc[position++] = proc;
        }
#endif
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        struct pcb_t * return_pcb;
#ifdef MLQ_SCHED
        //Skip the priority part
        
        if (empty(q)) return NULL;
        else{
                //Get the first pcb_t of the queue
                return_pcb = q->proc[0];
                //Shift the entire queue to the left by one element
                for (int i =0 ; i < q->size; i++){
                        q->proc[i] = q->proc[i+1];
                }
                //Reduce queue size
                q->size --;
                return return_pcb;
        }
#else
        if (empty(q)) return NULL;
        else{
                return_pcb = q->proc[0]
                for (int i =0 ; i < q->size; i++){
                        q->proc[i] = q->proc[i+1];
                }
        }
        q->size--;
        return return_pcb;
        
#endif   
	return NULL;
}

