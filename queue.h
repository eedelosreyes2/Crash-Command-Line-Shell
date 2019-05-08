#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>

/* Linked list node to store queue entry */ 
struct QNode { 
	struct history_entry *entry;
	struct QNode *next; 
}; 

/* Queue which stores front node of linked list and rear store last node */
struct Queue { 
	struct QNode *front, *rear; 
}; 

/* Function Prototypes */
struct QNode *newNode(struct history_entry *entry);
struct Queue *createQueue(void); 
void enQueue(struct Queue *q, struct history_entry *entry);
struct QNode *deQueue(struct Queue *q);
bool is_empty(struct Queue *q);
 
#endif
