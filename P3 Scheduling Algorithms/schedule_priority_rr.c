/**
 * schedule_priority_rr.c
 * Implements Priority Round Robin scheduling.
 * add(): enqueue tasks with metadata; schedule(): execute in priority-quantum slices.
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
 * Create a Task and append it to the scheduler's queue.
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

// compare for priority-order then lex
int compareTasks(const void *a, const void *b) {
    Task *ta = *(Task **)a;
    Task *tb = *(Task **)b;
    if (ta->priority != tb->priority) {
        return tb->priority - ta->priority;
    }
    return strcmp(ta->name, tb->name);
}

/**
 * schedule
 * Execute the Priority Round Robin scheduling loop until all tasks complete.
 * Records each task's start and finish times for metrics.
 */
void schedule() {
    // move tasks into array and sort by priority/name
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

    qsort(tasks, n, sizeof(Task *), compareTasks);

    int currentTime = 0;
    int remaining = n;
    while (remaining > 0) {
        // Determine highest priority among remaining tasks
        int maxPrio = -1;
        for (int i = 0; i < n; i++) {
            if (tasks[i] && tasks[i]->burst > 0 && tasks[i]->priority > maxPrio) {
                maxPrio = tasks[i]->priority;
            }
        }
        // Round-robin among tasks of highest priority
        bool groupRemaining = true;
        while (groupRemaining) {
            groupRemaining = false;
            for (int i = 0; i < n; i++) {
                if (tasks[i] && tasks[i]->priority == maxPrio && tasks[i]->burst > 0) {
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
                    } else {
                        groupRemaining = true;
                    }
                }
            }
        }
    }
}