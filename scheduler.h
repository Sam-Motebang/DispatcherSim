#ifndef SCHEDULER_H
#define SCHEDULER_H


#include "queue.h"
#include "process.h"

#define TIME_SLICE 3   /* Default RR quantum (ticks) */

/* Returns the next PCB from the front of the queue (FIFO). */
PCB *FCFS(Queue *readyQueue);

/* Returns the PCB with the shortest remaining burst time. */
PCB *SJF(Queue *readyQueue);


/*Round-Robin scheduler.*/
PCB *RoundRobin(Queue *readyQueue, PCB *runningProcess, int timeSlice);

/*Priority scheduler with Round-Robin tie-breaking.*/
PCB *PriorityRR(Queue *readyQueue, PCB *runningProcess, int timeSlice);

#endif
