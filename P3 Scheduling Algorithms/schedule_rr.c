/**
 * schedule_rr.c
 * Implements Round Robin scheduling.
 * add(): enqueue tasks in arrival order; schedule(): execute in fixed quantums.
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

#define QUANTUM 10

struct node *g_head = NULL;

/**
 * add
 * Create a Task and append it to the round-robin queue.
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
    // Append the new task to the end of the list to preserve arrival order
    struct node *new_node = malloc(sizeof(struct node));
    new_node->task = task;
    new_node->next = NULL;
    if (g_head == NULL) {
        g_head = new_node;
    } else {
        struct node *cur_node = g_head;
        while (cur_node->next != NULL) {
            cur_node = cur_node->next;
        }
        cur_node->next = new_node;
    }
}

/**
 * schedule
 * Execute the Round Robin scheduling loop until all tasks complete.
 * Records each task's start and finish times for metrics.
 */
void schedule() {
    // move tasks to an array for round-robin
    Task *tasks[100];
    int n = 0;
    struct node *cur = g_head;
    while (cur) {
        tasks[n++] = cur->task;
        struct node *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    g_head = NULL;

    int currentTime = 0;
    int remaining = n;
    while (remaining > 0) {
        for (int i = 0; i < n; i++) {
            if (tasks[i] && tasks[i]->burst > 0) {
                int slice = tasks[i]->burst > QUANTUM ? QUANTUM : tasks[i]->burst;
                if (metric_start[tasks[i]->tid] < 0) {
                    metric_start[tasks[i]->tid] = currentTime;
                }
                run(tasks[i], slice);
                tasks[i]->burst -= slice;
                currentTime += slice;
                if (tasks[i]->burst == 0) {
                    metric_finish[tasks[i]->tid] = currentTime;
                }
                printf("\tTime is now: %d\n", currentTime);
                if (tasks[i]->burst == 0) {
                    // free(tasks[i]->name);
                    free(tasks[i]);
                    tasks[i] = NULL;
                    remaining--;
                }
            }
        }
    }
}