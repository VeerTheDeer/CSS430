/**
 * task.h
 * Defines the Task struct for the scheduling simulation.
 * Tracks a task's name, ID, priority, burst length, and timing metrics.
 */
/**
 * Representation of a task in the system.
 */

#ifndef TASK_H
#define TASK_H

// representation of a task
typedef struct task {
    char *name;
    int tid;
    int priority;
    int burst;
    int original_burst;
    int arrival_time;
    int start_time;
    int finish_time;
} Task;

#endif
