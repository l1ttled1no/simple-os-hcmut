#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void printQueue(struct queue_t * q);

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (q->size < MAX_QUEUE_SIZE){
                q->proc[q->size] = proc;
                q->size++;
        }
}

int queuePeek (struct queue_t *q)
{
    int maxPrio = 9999;
    int index = -1;

    for (int i = 0; i < q->size; i++)
        {
            if (maxPrio > q->proc[i]->priority)
                {
                    maxPrio = q->proc[i]->priority;
                    index = i;
                }
        }

    return index;
}

struct pcb_t *
dequeue (struct queue_t *q)
{
    /* TODO: return a pcb whose priority is the highest
     * in the queue [q] and remember to remove it from q
     * */
#ifdef MLQ_SCHED // Skip the priority and dequeue the first proc if using MLQ

    if (!empty (q))
        {
            struct pcb_t *newProc = malloc (sizeof (struct pcb_t));
            newProc = q->proc[0];
            for (int i = 0; i < q->size; i++)
                {
                    q->proc[i] = q->proc[i + 1];
                }
            q->size--;
            return newProc;
        }

#else
    if (!empty (q))
        {
            int index = queuePeek (q);
            struct pcb_t *newProc = malloc (sizeof (struct pcb_t));
            newProc = q->proc[index];
            for (int i = index; i < q->size; i++)
                {
                    q->proc[i] = q->proc[i + 1];
                }
            q->size--;
            return newProc;
        }
#endif
    return NULL;
}
