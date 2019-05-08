#include "queue.h"
#include "history.h"

/**
 * Function to create a new linked list node 
 *
 * Parameters:
 * - entry: history entry to create new node
 *
 * Returns: QNode with new history entry.
 */
struct QNode *newNode(struct history_entry *entry) { 
	struct QNode *temp = (struct QNode*)malloc(sizeof(struct QNode)); 
	temp->entry = entry; 
	temp->next = NULL; 
	return temp; 
} 

/**
 * Function to create an empty linked list
 *
 * Parameters:
 * - void
 *
 * Returns: new queue.
 */
struct Queue *createQueue(void) { 
	struct Queue *q = (struct Queue*)malloc(sizeof(struct Queue)); 
	q->front = q->rear = NULL; 
	return q; 
} 

/**
 * Function to add a history entry to linked list 
 *
 * Parameters:
 * - q: queue to add history entry to
 * - entry: entry to add to queue
 *
 * Returns: void
 */
void enQueue(struct Queue *q, struct history_entry *entry) { 
	struct QNode *temp = newNode(entry); 

	/* If list is empty, then new node is front and rear both */
	if (q->rear == NULL) { 
		q->front = q->rear = temp; 
		return; 
	} 

	/* Add the new node at the end of list and change rear */
	q->rear->next = temp; 
	q->rear = temp; 
} 

/**
 * Function to remove a history entry from given list 
 *
 * Parameters:
 * - q: queue to dequeue from
 *
 * Returns: dequeued QNode.
 */
struct QNode *deQueue(struct Queue *q) { 
	/* Check if list is empty */
	if (q->front == NULL) {
		return NULL;
	}

	/* Store previous front and move front one node ahead */
	struct QNode *temp = q->front; 
	q->front = q->front->next; 

	/* If front becomes NULL, then change rear also as NULL */
	if (q->front == NULL) {
		q->rear = NULL; 
	}
	return temp; 
} 

/**
 * Function to return if given linked list is empty or not 
 *
 * Parameters:
 * - q: queue to check if empty
 *
 * Returns: true if empty, false if not empty.
 */
bool is_empty(struct Queue *q) {
	/* Check if list is empty */
	if (q->front == NULL) {
		return true;
	}
	return false;
}

/**
 * Function to print history entries in given list 
 *
 * Parameters:
 * - q: queue to print
 *
 * Returns: void
 */
void print_queue(struct Queue *q) {
	/* Check if list is empty */
	if (q->front == NULL) {
		return;
	}

	/* Traverse linked list */
	struct QNode *node = q->front;
	while (node != NULL) {
		printf("%d %s", node->entry->cmd_id, node->entry->line);
		node = node->next;
	}
}

