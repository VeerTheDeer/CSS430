/**
 * Various list operations
 */
/**
 * list.c
 * Utility functions for managing linked lists of Task nodes.
 * Provides insert, delete, and traverse operations.
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "task.h"


/**
 * insert
 * Insert a new Task node at the head of the list.
 * @param head      Pointer to list head pointer
 * @param newTask   Task to insert
 */
void insert(struct node **head, Task *newTask) {
    // add the new task to the list 
    struct node *newNode = malloc(sizeof(struct node));

    newNode->task = newTask;
    newNode->next = *head;
    *head = newNode;
}

/**
 * delete
 * Remove a specific Task from the list by matching name.
 * @param head      Pointer to list head pointer
 * @param task      Task to remove
 */
void delete(struct node **head, Task *task) {
    struct node *temp;
    struct node *prev;

    temp = *head;
    // special case - beginning of list
    if (strcmp(task->name,temp->task->name) == 0) {
        *head = (*head)->next;
    }
    else {
        // interior or last element in the list
        prev = *head;
        temp = temp->next;
        while (strcmp(task->name,temp->task->name) != 0) {
            prev = temp;
            temp = temp->next;
        }

        prev->next = temp->next;
    }
}

/**
 * traverse
 * Print all tasks in the list for debugging purposes.
 * @param head      Head of the list to traverse
 */
void traverse(struct node *head) {
    struct node *temp;
    temp = head;

    while (temp != NULL) {
        printf("[%s] [%d] [%d]\n",temp->task->name, temp->task->priority, temp->task->burst);
        temp = temp->next;
    }
}
