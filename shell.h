#ifndef _SHELL_H_
#define _SHELL_H_

#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

/* Preprocessor Directives */
#define ARG_MAX 4096
#define BUF_SZ 128

/* Struct to store command line information */
struct command_line {
    char **tokens;
    bool stdout_pipe;
    char *stdout_file;
};

/* Struct to store background job information */
struct job {
	pid_t pid;
	char *cmd;
};

/* Function Prototypes */
void execute(char *line);
int execute_pipeline(struct command_line *cmds);
bool builtin_cmd(char *tokens[], char *line);
void background_cmd(char *tokens[], pid_t pid);
void start_prompt(void);
void print_prompt(void);
bool startsWith(const char *pre, const char *str);
char *replace_str(char *str, char *orig, char *rep);
void delete_job(pid_t pid);

#endif
