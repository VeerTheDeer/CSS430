/**
 * schedule_priority.c
 * Implements Priority scheduling (non-preemptive).
 * add(): enqueue tasks with metadata; schedule(): execute by highest priority.
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
 * Create a Task and add it to the scheduler's list.
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
    insert(&g_head, task);
}

bool comesBefore(char *a, char *b) {
    return strcmp(a, b) < 0;
}

Task *pickNextTask() {
    if (!g_head) return NULL;

    struct node *temp = g_head;
    Task *best = temp->task;
    while (temp) {
        if (temp->task->priority > best->priority ||
           (temp->task->priority == best->priority && comesBefore(temp->task->name, best->name))) {
            best = temp->task;
        }
        temp = temp->next;
    }
    delete(&g_head, best);
    return best;
}

/**
 * schedule
 * Execute the Priority scheduling loop until all tasks complete.
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
            // Priority always runs to completion
            metric_finish[task->tid] = currentTime;
        }
        printf("\tTime is now: %d\n", currentTime);
        // free(task->name);
        free(task);
    }
}