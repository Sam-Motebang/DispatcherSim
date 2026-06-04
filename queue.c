#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

/* =========================================================
 * queue.c  –  Ready-queue implementation
 * ========================================================= */

/* Allocate and initialise an empty queue. */
Queue *createQueue(void) {
    Queue *q = (Queue *)malloc(sizeof(Queue));
    if (!q) { perror("createQueue: malloc failed"); exit(EXIT_FAILURE); }
    q->front = NULL;
    q->rear  = NULL;
    q->size  = 0;
    return q;
}

/* Append a PCB to the rear of the queue. */
void enqueue(Queue *q, PCB *process) {
    if (!q || !process) return;
    process->next = NULL;

    if (q->rear == NULL) {
        q->front = process;
        q->rear  = process;
    } else {
        q->rear->next = process;
        q->rear       = process;
    }
    q->size++;
}

/* Remove and return the front PCB; returns NULL if empty. */
PCB *dequeue(Queue *q) {
    if (!q || q->front == NULL) return NULL;

    PCB *temp  = q->front;
    q->front   = q->front->next;

    if (q->front == NULL)
        q->rear = NULL;

    temp->next = NULL;
    q->size--;
    return temp;
}

/* Return (but do not remove) the front PCB. */
PCB *peekFront(Queue *q) {
    if (!q) return NULL;
    return q->front;
}

/* 1 if queue is empty, 0 otherwise. */
int isEmpty(Queue *q) {
    return (!q || q->front == NULL);
}

/* Number of PCBs currently in the queue. */
int queueSize(Queue *q) {
    return q ? q->size : 0;
}

/* Print every PID in the queue (front → rear). */
void printQueue(Queue *q) {
    if (isEmpty(q)) {
        printf("  [queue empty]\n");
        return;
    }
    PCB *cur = q->front;
    printf("  [Queue front -> rear]: ");
    while (cur) {
        printf("P%d(rem:%d,pri:%d) ", cur->pid, cur->remainingTicks, cur->priority);
        cur = cur->next;
    }
    printf("\n");
}

/* Free the queue struct itself (PCBs should already have been freed). */
void destroyQueue(Queue *q) {
    if (!q) return;
    /* Free any PCBs still in the queue */
    PCB *cur = q->front;
    while (cur) {
        PCB *next = cur->next;
        free(cur);
        cur = next;
    }
    free(q);
}
