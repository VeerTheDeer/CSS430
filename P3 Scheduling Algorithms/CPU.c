/**
 * "Virtual" CPU that also maintains track of system time.
 */
/**
 * CPU.c
 * Simulates a virtual CPU with dispatch overhead tracking.
 * Maintains global counters for CPU time, dispatch time, and run count.
 */

#include <stdio.h>


#include "task.h"

// Bonus: counters for CPU utilization
int total_cpu_time = 0;
int total_dispatch_time = 0;
static int run_count = 0;

// Metrics for TAT, WT, RT
#define MAX_TASKS 100
int task_count = 0;
char *metric_names[MAX_TASKS];
int metric_arrival[MAX_TASKS];
int metric_start[MAX_TASKS];
int metric_finish[MAX_TASKS];
int metric_burst[MAX_TASKS];

/**
 * run
 * Simulate execution of a task slice on the CPU.
 * Applies a 1-unit dispatch overhead between task runs.
 * @param task  Pointer to Task being executed
 * @param slice Number of time units to run this task
 */
void run(Task *task, int slice) {
    // dispatch cost (1 unit) before each run after the first
    if (run_count > 0) {
        total_dispatch_time += 1;
    }
    total_cpu_time += slice;
    run_count++;
    printf("Running task = [%s] [%d] [%d] for %d units.\n",
           task->name, task->priority, task->burst, slice);
}
