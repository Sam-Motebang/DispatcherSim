#define _POSIX_C_SOURCE 199309L   /* expose clock_gettime, usleep, useconds_t */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   /* usleep() */
#include <time.h>     /* clock_gettime(), struct timespec */
#include "process.h"
#include "queue.h"
#include "scheduler.h"

/*
    1 000 000 us = 1 second
      100 000 us = 0.1 s
*/
#define TICK_DURATION_US  100000

/* Simulation table */
typedef struct {
    int id;
    int arriveAfterTick;
    int maxTicks;
    int priority;
} SimEntry;

SimEntry simulation[] = {
    {1, 0, 8,  2},   /* arrives tick 0, burst time =  8,  priority 2 */
    {2, 4, 2,  1},   /* arrives tick 2, burst time = 20, priority 1  */
    {3, 0, 20, 2},    /* arrives tick 0, burst time =  6,  priority 2  */
    {4, 5, 10, 3},
    {5, 2, 7, 10},
    {6, 3, 12, 6},
    {7, 10, 4, 2}
};
int simSize = 7;

/*Elapsed-time */
static struct timespec simStart;


static void startClock(void) {
    clock_gettime(CLOCK_MONOTONIC, &simStart);
}

/* Returns elapsed seconds since startClock() was called */
static double elapsedSeconds(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec  - simStart.tv_sec) +
           (now.tv_nsec - simStart.tv_nsec) / 1e9;
}

/* Context switch helper */
void contextSwitch(PCB **runningProcess, PCB *newProcess) {
    if (*runningProcess != NULL) {
        printf("  [CONTEXT SWITCH] P%d -> P%d\n",
               (*runningProcess)->pid,
               newProcess ? newProcess->pid : -1);
        (*runningProcess)->state         = READY;
        (*runningProcess)->timeSliceUsed = 0;
        (*runningProcess)->rrCounter     = 0;
    }
    *runningProcess = newProcess;
    if (*runningProcess != NULL) {
        (*runningProcess)->state = RUNNING;
        printf("  [DISPATCH] P%d is now RUNNING\n", (*runningProcess)->pid);
    }
}

/* Stats table */
void printStats(PCB **done, int count, int totalTicks, int contextSwitches,
                double wallTime) {
    printf("\n\n");
    printf("  PERFORMANCE STATISTICS\n");
    printf("\n");
    printf("%-5s %-8s %-7s %-12s %-12s %-10s\n",
           "PID", "Arrival", "Burst", "Completion", "Turnaround", "Waiting");
    printf("\n\n");

    double totalTAT = 0, totalWT = 0;
    for (int i = 0; i < count; i++) {
        PCB *p = done[i];
        printf("%-5d %-8d %-7d %-12d %-12d %-10d\n",
               p->pid, p->arrivalTime, p->maxTicks,
               p->completionTime, p->turnaroundTime, p->waitingTime);
        totalTAT += p->turnaroundTime;
        totalWT  += p->waitingTime;
    }
    printf("\n");
    printf("  Average Turnaround Time : %.2f ticks\n", totalTAT / count);
    printf("  Average Waiting Time    : %.2f ticks\n", totalWT  / count);
    printf("\n\n");
    printf("  Total ticks      : %d\n", totalTicks);
    printf("  Context switches : %d\n", contextSwitches);
    printf("  Wall-clock time  : %.2f s\n", wallTime);
    printf("\n\n");
}

void runSimulation(int algorithm) {
    Queue *readyQueue     = createQueue();
    PCB   *runningProcess = NULL;

    int timeSlice      = 2;
    int tick           = 0;
    int finished       = 0;
    int contextSwitches = 0;

    /* Storage for finished PCBs (for stats) */
    PCB **doneList = (PCB **)malloc(simSize * sizeof(PCB *));
    int   doneCount = 0;

    printf("\nStarting simulation with algorithm: ");
    switch (algorithm) {
        case 1: printf("FCFS\n");                                break;
        case 2: printf("Round Robin (quantum: %d)\n", timeSlice); break;
        case 3: printf("Priority Scheduling (RR tie-break)\n");  break;
        case 4: printf("Shortest Job First\n");                  break;
    }
    printf("\n\n");
    printf("  Tick duration    : 1 second \n");
    printf("\n\n");

    startClock();   /*begin wall-clock measurement */

    while (finished < simSize) {
        double t0 = elapsedSeconds();   /* tick start time */

        printf("--- Tick %d  (elapsed: %.1f s) ---\n", tick, t0);

        /* 1. Check arrivals */
        for (int i = 0; i < simSize; i++) {
            if (simulation[i].arriveAfterTick == tick) {
                PCB *p = (PCB *)malloc(sizeof(PCB));
                p->pid            = simulation[i].id;
                p->arrivalTime    = tick;
                p->maxTicks       = simulation[i].maxTicks;
                p->remainingTicks = simulation[i].maxTicks;
                p->priority       = simulation[i].priority;
                p->state          = READY;
                p->waitingTime    = 0;
                p->turnaroundTime = 0;
                p->completionTime = 0;
                p->hasStarted     = 0;
                p->rrCounter      = 0;
                p->timeSliceUsed  = 0;
                p->next           = NULL;

                enqueue(readyQueue, p);
                printf("  [ARRIVAL] P%d arrived (burst=%d, priority=%d)\n",
                       p->pid, p->maxTicks, p->priority);
            }
        }

        /* 2. Increment waiting time for every queued process */
        {
            PCB *cur = readyQueue->front;
            while (cur) { cur->waitingTime++; cur = cur->next; }
        }

        /* 3. Select next process */
        PCB *nextProcess = NULL;
        switch (algorithm) {
            case 1: /* FCFS - non-preemptive */
                if (runningProcess == NULL)
                    nextProcess = FCFS(readyQueue);
                else
                    nextProcess = runningProcess;
                break;
            case 2: /* Round Robin - preemptive */
                if (runningProcess != NULL) runningProcess->rrCounter++;
                nextProcess = RoundRobin(readyQueue, runningProcess, timeSlice);
                break;
            case 3: /* Priority + RR - preemptive */
                if (runningProcess != NULL) runningProcess->rrCounter++;
                nextProcess = PriorityRR(readyQueue, runningProcess, timeSlice);
                break;
            case 4: /* SJF - non-preemptive */
                if (runningProcess == NULL)
                    nextProcess = SJF(readyQueue);
                else
                    nextProcess = runningProcess;
                break;
        }

        /* 4. Context switch if process changed */
        if (nextProcess != NULL && nextProcess != runningProcess) {
            if (runningProcess != NULL) contextSwitches++;
            contextSwitch(&runningProcess, nextProcess);
        } else if (runningProcess == NULL && nextProcess == NULL) {
            printf("  [IDLE] No process ready\n");
        }

        /* 5. Execute one tick */
        if (runningProcess != NULL) {
            runningProcess->remainingTicks--;
            runningProcess->timeSliceUsed++;

            if (algorithm == 2 || algorithm == 3) {
                printf("  [RUNNING] P%d  remaining=%d  slice=%d/%d  waiting=%d\n",
                       runningProcess->pid,
                       runningProcess->remainingTicks,
                       runningProcess->timeSliceUsed,
                       timeSlice,
                       runningProcess->waitingTime);
            } else {
                printf("  [RUNNING] P%d  remaining=%d  waiting=%d\n",
                       runningProcess->pid,
                       runningProcess->remainingTicks,
                       runningProcess->waitingTime);
            }

            /* 6. Check completion */
            if (runningProcess->remainingTicks == 0) {
                runningProcess->completionTime = tick + 1;
                runningProcess->turnaroundTime =
                    runningProcess->completionTime - runningProcess->arrivalTime;
                runningProcess->waitingTime =
                    runningProcess->turnaroundTime - runningProcess->maxTicks;
                runningProcess->state = TERMINATED;

                printf("  [FINISHED] P%d  completion=%d  TAT=%d  WT=%d\n",
                       runningProcess->pid,
                       runningProcess->completionTime,
                       runningProcess->turnaroundTime,
                       runningProcess->waitingTime);

                doneList[doneCount++] = runningProcess;
                runningProcess = NULL;
                finished++;
            }
        }

        /* ---- Real-time delay: sleep for the remainder of this tick ---- */
        double elapsed      = elapsedSeconds() - t0;    /* work took this long */
        long   remaining_us = TICK_DURATION_US - (long)(elapsed * 1e6);
        if (remaining_us > 0) {
            struct timespec ts;
            ts.tv_sec  = remaining_us / 1000000L;
            ts.tv_nsec = (remaining_us % 1000000L) * 1000L;
            nanosleep(&ts, NULL);                        /* sleep the rest */
        }

        printf("\n");
        tick++;
    }

    double totalWall = elapsedSeconds();

    printf("\n\n");
    printf("  All processes completed.\n");
    printf("\n\n");

    printStats(doneList, doneCount, tick, contextSwitches, totalWall);

    /* Cleanup */
    for (int i = 0; i < doneCount; i++) free(doneList[i]);
    free(doneList);
    destroyQueue(readyQueue);
}

/* Entry point*/
int main(int argc, char *argv[]) {
    int choice;

    printf("CPU Scheduling Simulator\n");
    printf("\n\n");

    if (argc >= 2) {
        /* Algorithm passed as command-line argument (e.g. make run1) */
        choice = atoi(argv[1]);
        if (choice < 1 || choice > 4) {
            fprintf(stderr, "Invalid argument '%s'. Pass 1-4.\n", argv[1]);
            return 1;
        }
    } else {
        /* Interactive mode */
        printf("Select scheduling algorithm:\n");
        printf("  1. First-Come-First-Serve (FCFS)\n");
        printf("  2. Round Robin \n");
        printf("  3. Priority Scheduling (RR tie-breaking)\n");
        printf("  4. Shortest Job First (SJF)\n");
        printf("Enter your choice (1-4): ");

        while (scanf("%d", &choice) != 1 || choice < 1 || choice > 4) {
            printf("Invalid input. Please enter 1, 2, 3, or 4: ");
            while (getchar() != '\n');
        }
    }

    runSimulation(choice);
    return 0;
}
