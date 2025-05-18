/**
 * Driver.c
 *
 * Schedule is in the format
 *
 *  [name] [priority] [CPU burst]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "schedulers.h"
#include "cpu.h"
extern int total_cpu_time;
extern int total_dispatch_time;

extern int task_count;
extern char *metric_names[];
extern int metric_arrival[];
extern int metric_start[];
extern int metric_finish[];
extern int metric_burst[];

#define SIZE    100

/**
 * main
 * Entry point for scheduling simulation.
 * Opens the input file, parses tasks, runs the scheduler,
 * and outputs CPU utilization and task metrics.
 * @param argc Number of command-line arguments
 * @param argv Array of argument strings (argv[1] is task file)
 * @return Exit status (0 for success)
 */
int main(int argc, char *argv[])
{
    FILE *in;
    char *temp;
    char task[SIZE];

    char *name;
    int priority;
    int burst;

    in = fopen(argv[1],"r");
    // Open the task definition file for reading
    
    while (fgets(task,SIZE,in) != NULL) {
        temp = strdup(task);
        name = strsep(&temp,",");
        priority = atoi(strsep(&temp,","));
        burst = atoi(strsep(&temp,","));

        // add the task to the scheduler's list of tasks
        add(name,priority,burst);

        free(temp);
    }

    // Close the input file
    fclose(in);

    // invoke the scheduler
    schedule();

    // output CPU utilization including dispatcher cost
    double util = (double)total_cpu_time * 100.0 /
                  (total_cpu_time + total_dispatch_time);
    printf("CPU Utilization: %.2f%%\n", util);

    // Print table header with task names
    printf("\n...|");
    for (int i = 0; i < task_count; i++) {
        printf(" %2s |", metric_names[i]);
    }
    printf("\n");
    // Print Turn-Around Time for each task
    printf("TAT|");
    for (int i = 0; i < task_count; i++) {
        int tat = metric_finish[i] - metric_arrival[i];
        printf(" %2d |", tat);
    }
    printf("\n");
    // Print Waiting Time for each task
    printf("WT |");
    for (int i = 0; i < task_count; i++) {
        int wt = (metric_finish[i] - metric_arrival[i]) - metric_burst[i];
        printf(" %2d |", wt);
    }
    printf("\n");
    // Print Response Time for each task
    printf("RT |");
    for (int i = 0; i < task_count; i++) {
        int rt = metric_start[i] - metric_arrival[i];
        printf(" %2d |", rt);
    }
    printf("\n");

    return 0;
}
