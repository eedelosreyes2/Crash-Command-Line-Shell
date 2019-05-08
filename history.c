#include "history.h"
#include "shell.h"
#include "queue.h"
#include "tokenizer.h"

/* Globals */
struct Queue *history;
int size;

/**
 * Function to set up history queue 
 *
 * Parameters:
 * - void
 *
 * Returns: void
 */
void init_history(void) {
	history = createQueue();
	size = 0;
}

/** 
 * Function to add command entry to history of command entries 
 *
 * Parameters:
 * - cmd_id: cmd number to add to history
 * - line: line to add to history
 *
 * Returns: void
 */
void add_history(int cmd_id, char *line) {
	/* If not valid, then don't add to history */
	if (strcmp(line, "") == 0) {
		return;
	}
	
	/* Else if line starts with !!, get last command */
	else if (strncmp(line, "!!", strlen("!!")) == 0) {
		struct history_entry *temp = (struct history_entry*)malloc(sizeof(struct history_entry));
		temp = get_last_entry();
		/* Check if last entry exists */
		if (temp != NULL) {
			strcpy(line, temp->line);
		} else {
			return;
		}
	}

	/* Else if line starts with !, get line without ! */
	else if (startsWith("!", line)) {
		line = next_token(&line, "!");

		/* Check if argument is cmd id */
		int cmd_id = atoi(line);
		if (cmd_id > 0) {
			struct history_entry *temp = (struct history_entry*)malloc(sizeof(struct history_entry));
			temp = get_entry(cmd_id);
			if (temp != NULL) {
				strcpy(line, temp->line);
			}
		}

		/* If argument is line */
		else {
			int found = 1;
			struct history_entry *temp = (struct history_entry*)malloc(sizeof(struct history_entry));
			temp = get_entry_by_line(line, &found);
			/* If found latest command */
			if (found == 0) {
				strcpy(line, temp->line);
			}
		}
	}

	/* If history is at max size, then remove first in list */
	if (size >= HIST_MAX) {
		deQueue(history);
		size--;
	}

	/* Create history entry and add to list */
	struct history_entry *temp = (struct history_entry*)malloc(sizeof(struct history_entry));
	temp->cmd_id = cmd_id;
	temp->line = line;
	enQueue(history, temp);
	size++;
}

/**
 * Function to get history entry by cmd id 
 *
 * Parameters:
 * - cmd_id: cmd id to get history entry
 *
 * Returns: struct by cmd id.
 */
struct history_entry *get_entry(int cmd_id) {
	/* Check if cmd id is not accessible */
	if (cmd_id < size - 100) {
		perror("Event not found\n");
		return NULL;
	}

	/* Traverse history entries */
	struct QNode *node = history->front;
	while (node != NULL) {
		/* If cmd id matches, return entry */
		if (cmd_id == node->entry->cmd_id) {
			return node->entry;
		}
		node = node->next;
	}

	/* If not found, return null */
	return NULL;
}

/**
 * Function to get last history entry by line 
 *
 * Parameters:
 * - line: line to get history entry
 * - found: int to determine if history entry was found
 *
 * Returns: struct history entry with cmd id.
 */
struct history_entry *get_entry_by_line(char *line, int *found) {
	struct history_entry *latest = (struct history_entry*)malloc(sizeof(struct history_entry));
	int i = 1;

	/* Traverse history entries */
	struct QNode *node = history->front;
	while (node != NULL && i < size) {
		char *history_line = node->entry->line;
		history_line[strlen(history_line)-1] = '\0';
		/* If line matches, set to latest entry */
		if (startsWith(line, history_line)) {
			latest = node->entry;
			*found = 0;
		}
		node = node->next;
		i++;
	}

	return latest;
}

/**
 * Function to get last history entry in list 
 *
 * Parameters:
 * - void
 *
 * Returns: last struct history entry in history list.
 **/
struct history_entry *get_last_entry(void) {
	/* If not null, return last entry in list */
	if (history->rear != NULL) {
		if (history->rear->entry != NULL) {
			return history->rear->entry;
		}
	}

	/* If nothing in list, return null */
	return NULL;
}

/**
 * Function to print history entries 
 *
 * Parameters: 
 * - void
 *
 * Returns: void
 */
void print_history(void) {
	/* Traverse history entries */
	struct QNode *node = history->front;
	while (node != NULL) {
		/* Get history info and print to shell */
		int cmd_id = node->entry->cmd_id;
		char *line = node->entry->line;
		printf("%d %s", cmd_id, line);
		node = node->next; 
	}
}

