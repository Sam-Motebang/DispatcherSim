#ifndef PROCESS_H
#define PROCESS_H


typedef enum { READY, RUNNING, TERMINATED } ProcessState;

typedef struct PCB {
    int pid;               /* Unique process identifier          */
    int arrivalTime;       /* Tick at which the process arrives  */
    int maxTicks;          /* Total CPU ticks needed (burst time)*/
    int remainingTicks;    /* Ticks still left to execute        */
    int priority;          /* Lower number = higher priority     */
    int waitingTime;       /* Total ticks spent in ready queue   */
    int turnaroundTime;    /* completionTime - arrivalTime       */
    int completionTime;    /* Tick at which process finished     */
    int hasStarted;        /* Flag: 1 if process has run once    */
    int rrCounter;         /* Ticks used in current RR slice     */
    int timeSliceUsed;     /* Alias for rrCounter (RR display)   */
    ProcessState state;    /* Current state of the process       */
    struct PCB *next;      /* Pointer used by linked-list queue  */
} PCB;

#endif /* PROCESS_H */
