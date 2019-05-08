#ifndef _HISTORY_H_
#define _HISTORY_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/* Preprocessor Directives */
#define HIST_MAX 100
#define BUF_SZ 128

/* Struct to store information of each command entry */
struct history_entry {
	int cmd_id;
	char *line;
};

/* Function Prototypes */
void init_history(void);
void add_history(int cmd_id, char *line);
struct history_entry *get_entry(int cmd_id);
struct history_entry *get_entry_by_line(char *line, int *found);
struct history_entry *get_last_entry(void);
void print_history(void);

#endif
