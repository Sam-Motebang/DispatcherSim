#ifndef QUEUE_H
#define QUEUE_H

/* =========================================================
 * queue.h  –  Ready-queue interface
 *
 * A singly-linked FIFO queue of PCB pointers.
 * The PCB's own `next` field is used as the link, so no
 * separate node wrapper is required.
 * ========================================================= */

#include "process.h"

typedef struct Queue {
    PCB *front;   /* Head of the list (next to be dequeued) */
    PCB *rear;    /* Tail of the list (last enqueued)       */
    int  size;    /* Current number of entries              */
} Queue;

Queue *createQueue(void);
void   enqueue(Queue *q, PCB *process);
PCB   *dequeue(Queue *q);
PCB   *peekFront(Queue *q);
int    isEmpty(Queue *q);
int    queueSize(Queue *q);
void   printQueue(Queue *q);        /* Debug helper */
void   destroyQueue(Queue *q);      /* Free all remaining PCBs */

#endif /* QUEUE_H */
