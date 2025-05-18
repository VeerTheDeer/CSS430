/**
 * schedule_fcfs.c
 * Implements First-Come, First-Served scheduling.
 * add(): enqueue tasks in arrival order; schedule(): execute in queue order.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "task.h"
#include "list.h"
#include "schedulers.h"
#include "cpu.h"

extern int task_count;
extern char *metric_names[];
extern int metric_arrival[];
extern int metric_start[];
extern int metric_finish[];
extern int metric_burst[];

struct node *g_head = NULL;

/**
 * add
 * Create a Task and enqueue it for FCFS execution.
 * Initializes timing metrics for later reporting.
 */
void add(char *name, int priority, int burst) {
    Task *task = malloc(sizeof(Task));
    task->name = strdup(name);
    task->priority = priority;
    task->burst = burst;
    task->original_burst = burst;
    task->arrival_time = 0;
    task->start_time = -1;
    task->finish_time = -1;
    task->tid = task_count;
    metric_names[task_count] = task->name;
    metric_arrival[task_count] = 0;
    metric_burst[task_count] = burst;
    metric_start[task_count] = -1;
    metric_finish[task_count] = -1;
    task_count++;
    // Append to end of the list for FCFS order
    struct node *new_node = malloc(sizeof(struct node));
    new_node->task = task;
    new_node->next = NULL;
    if (g_head == NULL) {
        g_head = new_node;
    } else {
        struct node *cur = g_head;
        while (cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = new_node;
    }
}

// FCFS: always run the task at the head of the list
Task *pickNextTask() {
    if (g_head == NULL) {
        return NULL;
    }
    Task *task = g_head->task;
    // remove the head node
    struct node *old_head = g_head;
    g_head = g_head->next;
    free(old_head);
    return task;
}

/**
 * schedule
 * Execute the FCFS scheduling loop until all tasks complete.
 * Records each task's start and finish times for metrics.
 */
void schedule() {
    Task *task;
    int currentTime = 0;
    while ((task = pickNextTask()) != NULL) {
        if (metric_start[task->tid] < 0) {
            metric_start[task->tid] = currentTime;
        }
        run(task, task->burst);
        currentTime += task->burst;
        if (task->burst == 0) {
            metric_finish[task->tid] = currentTime;
        }
        else if (task->burst > 0) {
            // FCFS always runs to completion
            metric_finish[task->tid] = currentTime;
        }
        printf("\tTime is now: %d\n", currentTime);
        // free(task->name);
        free(task);
    }
}