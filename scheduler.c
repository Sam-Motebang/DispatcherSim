#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

/* 
  1. FCFS – First-Come, First-Served
     Simply dequeues and returns the front of the ready queue.
     Non-preemptive: the selected process runs to completion.
 */
PCB *FCFS(Queue *readyQueue) {
    if (isEmpty(readyQueue))
        return NULL;
    return dequeue(readyQueue);
}

/*
  2. SJF – Shortest Job First (non-preemptive)
     Scans the entire queue and picks the PCB with the
     smallest remainingTicks.  Ties are broken by arrival
     time, then by PID.
 */
PCB *SJF(Queue *readyQueue) {
    if (isEmpty(readyQueue))
        return NULL;

    /* Find the PCB with the shortest burst */
    PCB *best     = NULL;
    PCB *bestPrev = NULL;
    PCB *cur      = readyQueue->front;
    PCB *prev     = NULL;

    while (cur) {
        if (best == NULL
            || cur->remainingTicks < best->remainingTicks
            || (cur->remainingTicks == best->remainingTicks
                && cur->arrivalTime < best->arrivalTime)
            || (cur->remainingTicks == best->remainingTicks
                && cur->arrivalTime == best->arrivalTime
                && cur->pid < best->pid)) {
            best     = cur;
            bestPrev = prev;
        }
        prev = cur;
        cur  = cur->next;
    }

    /* Unlink best from  the queue */
    if (bestPrev == NULL) {
        /* best is at the front */
        readyQueue->front = best->next;
    } else {
        bestPrev->next = best->next;
    }
    if (best == readyQueue->rear) {
        readyQueue->rear = bestPrev;
    }
    best->next = NULL;
    readyQueue->size--;

    return best;
}

/*
  3. Round-Robin (preemptive)
     Each process gets at most `timeSlice` consecutive ticks.
     When the slice expires the running process is placed at
     the rear of the queue and the front process is selected.
 
     The caller must:
       a) NOT free runningProcess before calling this.
       b) Replace its runningProcess pointer with the return
          value.
     If the running process is preempted this function
     enqueues it internally.
 */
PCB *RoundRobin(Queue *readyQueue, PCB *runningProcess, int timeSlice) {

    /* If something is already running, decide whether to preempt */
    if (runningProcess != NULL) {
        if (runningProcess->rrCounter >= timeSlice) {
            /* Slice expired – put it back and pick next */
            runningProcess->rrCounter = 0;
            enqueue(readyQueue, runningProcess);
            runningProcess = NULL;
        } else {
            /* Still within its slice – let it continue */
            return runningProcess;
        }
    }

    /* Nothing running (or was just preempted) – pick front of queue */
    if (isEmpty(readyQueue))
        return NULL;

    return dequeue(readyQueue);
}

/*
  4. Priority Scheduling with Round-Robin tie-breaking
     (preemptive)

     At each call the function finds the highest-priority
     level present in the queue.  If the running process
     already has that priority it is allowed to continue
     for up to `timeSlice` ticks.  If a higher-priority process
     has arrived the running process
     is preempted immediately (its rrCounter resets so it
     gets a fresh slice next time it runs).
  */
PCB *PriorityRR(Queue *readyQueue, PCB *runningProcess, int timeSlice) {

    if (isEmpty(readyQueue)) {
        /* Nothing in queue – keep running the current process if any */
        return runningProcess;
    }

    /* Find the best (lowest) priority value in the ready queue */
    int bestPriority = readyQueue->front->priority;
    PCB *cur = readyQueue->front->next;
    while (cur) {
        if (cur->priority < bestPriority)
            bestPriority = cur->priority;
        cur = cur->next;
    }

    /* If something is running, compare its priority */
    if (runningProcess != NULL) {
        if (runningProcess->priority < bestPriority) {
            /* Running process has strictly higher priority – keep running */
            return runningProcess;
        }
        if (runningProcess->priority == bestPriority) {
            /* Same priority level – apply RR */
            if (runningProcess->rrCounter < timeSlice) {
                return runningProcess;   /* Still within its slice */
            }
            /* Slice expired – preempt and re-enqueue */
            runningProcess->rrCounter = 0;
            enqueue(readyQueue, runningProcess);
            runningProcess = NULL;
        } else {
            /* A higher-priority process has arrived – preempt immediately */
            runningProcess->rrCounter = 0;
            enqueue(readyQueue, runningProcess);
            runningProcess = NULL;
        }
    }

    /* Pick the highest-priority process from the queue.
     * Among equals keep FIFO order (first one found wins). */
    PCB *chosen     = NULL;
    PCB *chosenPrev = NULL;
    cur             = readyQueue->front;
    PCB *prev       = NULL;

    while (cur) {
        if (chosen == NULL || cur->priority < chosen->priority) {
            chosen     = cur;
            chosenPrev = prev;
        }
        prev = cur;
        cur  = cur->next;
    }

    /* Unlink chosen */
    if (chosenPrev == NULL) {
        readyQueue->front = chosen->next;
    } else {
        chosenPrev->next = chosen->next;
    }
    if (chosen == readyQueue->rear) {
        readyQueue->rear = chosenPrev;
    }
    chosen->next = NULL;
    readyQueue->size--;

    return chosen;
}
