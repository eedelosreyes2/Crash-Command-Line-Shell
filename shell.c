#include "debug.h"
#include "history.h"
#include "tokenizer.h"
#include "shell.h"

/* Globals */
int cmd_id = 0, jobs_i = 0;
char username[BUF_SZ], hostname[HOST_NAME_MAX], home_dir[PATH_MAX], cwd[PATH_MAX];
struct job *jobs[10];
bool command_executing;

/* Signal handler to handle ^C */
void sigint_handler(int signo) {
	if (isatty(STDIN_FILENO)) {
		printf("\n");
		if (!command_executing) {
			print_prompt();
		}
		fflush(stdout);
	}
}

/* Signal handler to handle child process exiting */
void sigchild_handler(int signo) {
	/* Child pid */
	pid_t kidpid;
	int status;
	while ((kidpid = waitpid(-1, &status, WNOHANG)) > 0) {
		/* Terminate child process with pid */
		delete_job(kidpid);
	}
	LOG("Child exited. Status: %d\n", status);
}

int main(void) {
	/* Initialize history */
	init_history();

	/* Initiate prompt */
	start_prompt();

	/* Set up signal handler */
	signal(SIGINT, sigint_handler);	

	/* Loop forever, prompting the user for commands */
	while (true) {
		/* If fd refers to terminal, show prompt */
		if (isatty(STDIN_FILENO)) {
			print_prompt();
		}

		char *line = NULL, *history = NULL;
		size_t line_sz = 0;
		size_t sz = getline(&line, &line_sz, stdin);

		/* Break if getline() fails */
		if (sz == EOF) {
			break;
		}

		LOG("-> Got line: %s", line);

		/* Before tokenizing line, copy for history entry */
		history = strdup(line); //dont forget to FREE

		/* Execute command */
		execute(line);

		/* Add command to history */
		add_history(cmd_id, history);

		/* Increment cmd id after execution */
		cmd_id++;
	}
	
	/* Clean up memory (and stuff) */
    return 0;
}

/**
 *  Function to execute command.
 *
 *	Parameters:
 *	- line: string to tokenize, then execute
 *
 *	Returns: void
 */
void execute(char *line) {
	char *tokens[ARG_MAX], *next_tok = strdup(line), *curr_tok;
	int i = 0, background = 0;

	/* Tokenize */
    while (((curr_tok = next_token(&next_tok, " \'\"\t\r\n")) != NULL) && i < ARG_MAX) {
		/* Allow comments with # */
		if (curr_tok[0] == '#') {
			break;
		}
		/* & acts as a command separator, run what came before that in background */
		if (curr_tok[0] == '&') {
			background = 1;
			break;
		}
		
		/* Expand environment variables */
		char *new_str = expand_var(curr_tok);
		if (new_str != NULL) {
			while (strstr(new_str, "$") != NULL) {
				new_str = expand_var(new_str);
			}
			curr_tok = new_str;
		}

		tokens[i++] = curr_tok;
	}
	tokens[i] = (char *) NULL;
	
	/* Check if argument is a built in command first */
	if (builtin_cmd(tokens, line)) {
		return;
	}

	/* Implement piping */
	struct command_line cmds[i];
	cmds[0].tokens = tokens;
	cmds[0].stdout_pipe = true;
	cmds[0].stdout_file = NULL;
	/* Keep track of command index and current token */
	int cmds_i = 1, curr_tok_i;
	/* Traverse tokens */
	for (curr_tok_i = 0; curr_tok_i < i; curr_tok_i++) {
		/* Find pipe */
		if (strcmp(tokens[curr_tok_i], "|") == 0) {
			/* Set pipe to null so tokenizer knows where to split */
			tokens[curr_tok_i] = NULL;
			/* Set up command line struct */
			cmds[cmds_i].tokens = &tokens[curr_tok_i + 1];
			cmds[cmds_i].stdout_pipe = true;
			cmds[cmds_i].stdout_file = NULL;
			cmds_i++;
		 } 
		 /* Find > operator */
		 else if (strcmp(tokens[curr_tok_i], ">") == 0) {
		 	/* Go to last command to edit */
			cmds_i--;
			/* Set > operator to null so tokenizer knows where to split */
			tokens[curr_tok_i] = NULL;
			/* Change stdout)file to token after > operator */
			cmds[cmds_i].stdout_file = tokens[curr_tok_i + 1];
			cmds_i++;
			break;
		 }
	}
	/* Last command so set stdout_pipe = false */
	cmds[cmds_i - 1].stdout_pipe = false;
	
	command_executing = true;
	pid_t pid = fork();
	if (pid == 0) {
		/* Child */
		/* Execute pipeline */
		int ret = execute_pipeline(cmds);
		fclose(stdin);
		if (ret == -1) {
			/* Error, exit */
			exit(0);
		}
	} else if (pid == -1) {
		perror("fork");	
	} else {
		/* Parent */
		/* If there are background jobs, add them to jobs list */
		if (background) {
			background_cmd(tokens, pid);
		} else {
			int status;
			waitpid(pid, &status, 0);
			command_executing = false;
			LOG("Child exited. Status: %d\n", status);
		}
	}
}

/* 
 * Function to execute command through pipeline.
 *
 * Parameters:
 * - cmds: command structs to run in pipe
 * 			
 * Returns: int, 1 if successful, 0 if unsuccessful.
 */
int execute_pipeline(struct command_line *cmds) {
	if (cmds->stdout_pipe == false) {
		if (cmds->stdout_file != NULL) {
			/**
			 * For a list of flags, see man 2 open:
			 *
			 * - O_RDWR - open for reading and writing (we could get by with O_WRONLY
			 *            instead)
			 * - O_CREAT - create file if it does not exist
			 * - O_TRUNC - truncate size to 0
			 */
			int open_flags = O_RDWR | O_CREAT | O_TRUNC;
			int open_perms = 0644;

			/* Create file descriptor with perms */
			int fd = open(cmds->stdout_file, open_flags, open_perms);
			/* If failed, print error */
			if (fd == -1) {
				perror("open");
				return 0;
			}
			/* If dup2 to stdout fails, print error */
			if (dup2(fd, fileno(stdout)) == -1) {
				perror("dup2");
				return 0;
			}	
		}
		/* If no error, execvp tokens */
		execvp(cmds->tokens[0], cmds->tokens);
		return 1;
	}

   /* Creates a pipe. */
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return 0;
    }

    pid_t pid = fork();
    if (pid == 0) {
        /* Child */
		dup2(fd[1], fileno(stdout));
        close(fd[0]);
		execvp(cmds->tokens[0], cmds->tokens);
    } else {
        /* Parent */
		dup2(fd[0], fileno(stdin));
		close(fd[1]);
		execute_pipeline(cmds + 1);
    } 
	return 0;
}

/**
 * Function to allow the shell to support built in functions that execvp() cannot
 *
 * Parameters:
 * - tokens: tokens to execute
 * - line: line to add to history
 * 			
 * Returns: true if successful, false if unsuccessful.
 */
bool builtin_cmd(char *tokens[], char *line) {
	/* If no tokens, return null */
	if (tokens[0] == NULL) {
		return false;
	}
	/* "cd" */
	if (strcmp(tokens[0], "cd") == 0) {
		/* Check if second argument is given */
		if (tokens[1] != NULL) {
			/* If given, check if directory exists */
			if (chdir(tokens[1]) == 0) {
				LOG("Switched directories from %s to %s successfully\n", cwd, tokens[1]);
			} else {
				LOG("Could not switch directories from %s to %s\n", cwd, tokens[1]);
			}
		/* If no second argument, switch to home directory */
		} else {
			chdir(home_dir);
			LOG("Swtiched directories from %s to %s successfully\n", cwd, home_dir);
		}
		return true;
	}
	/* "history" */
	if (strcmp(tokens[0], "history") == 0) {
		/* Add 'history' to history */
		add_history(cmd_id, line);
		print_history();
		return true;
	}
	/* "!!" */
	if (strcmp(tokens[0], "!!") == 0) {
		struct history_entry *temp = (struct history_entry*)malloc(sizeof(struct history_entry));
		temp = get_last_entry();
		/* Check if last entry exists */
		if (temp != NULL) {
			execute(temp->line);
		} else {
			perror("No last entry found\n");
		}
		return true;
	}
	/* "!" */
	else if (startsWith("!", line)) {
		/* Get line without ! */
		line = next_token(&line, "!");

		/* Check if argument is cmd id */
		int cmd_id = atoi(line);
		if (cmd_id > 0) {
			struct history_entry *temp = (struct history_entry*)malloc(sizeof(struct history_entry));
			temp = get_entry(cmd_id);
			if (temp != NULL) {
				execute(temp->line);
			}
		}

		/* If argument is line */
		else {
			int found = 1;
			struct history_entry *temp = (struct history_entry*)malloc(sizeof(struct history_entry));
			temp = get_entry_by_line(line, &found);
			/* If found latest command */
			if (found == 0) {
				execute(temp->line);
			} else {
				perror("No last entry found\n");
			}
		}
		return true;
	}
	/* "setenv" */
	if (strcmp(tokens[0], "setenv") == 0) {
		/* Check if there are enough commands, then setenv */
		if (tokens[2] != NULL) {
			setenv(tokens[1], tokens[2], true);
		}
	}
	/* "jobs" */
	if (strcmp(tokens[0], "jobs") == 0) {
		/* Iterate list of jobs and print */
		int i;
		for (i = 0; i < jobs_i; ++i) {
			printf("%d %s", jobs[i]->pid, jobs[i]->cmd);
		}
		return true;
	}
	/* "exit" */
	if (strcmp(tokens[0], "exit") == 0) {
		exit(0);
		return true;
	}
	return false;
}

/**
 * Function to allow the shell to support background jobs
 *
 * Parameters:
 * - tokens: tokens to execvp in the background
 * - pid: pid to add to jobs
 *
 * Returns: void
 */
void background_cmd(char *tokens[], pid_t pid) {
	/* Set up signal handler */
	signal(SIGCHLD, sigchild_handler);
	
	/* Create new job */
	struct job *new_job = (struct job*)malloc(sizeof(struct job));
	new_job->pid = pid;
	new_job->cmd = (char *)NULL;

	/* Tokenize line of new job and add */
	int i = 0;
	char *cmd = NULL;
	while (tokens[i] != NULL) {
		if (i == 0) {
			cmd = strdup(tokens[i]);
		} else {
			strcat(cmd, tokens[i]);
		}
		i++;
		strcat(cmd, " ");
	}
	
	/* If no line, do not add to list and return */
	if (cmd == NULL) {
		return;
	}

	/* Else, complete jobs then add to jobs list*/
	strcat(cmd, "&\n");
	new_job->cmd = cmd;
	jobs[jobs_i] = (struct job*)malloc(sizeof(struct job));
	jobs[jobs_i] = new_job;
	jobs_i++;
}

/** 
 * Function to initiate prompt and cache information
 *
 * Parameters:
 * - void
 *
 * Returns: void
 */
void start_prompt(void) {
	/* Get information from getpwuid */
	struct passwd *passwd;
	passwd = getpwuid(getuid());

	/* Get username */
	strcpy(username, passwd->pw_name);

	/* Get hostname */
	hostname[HOST_NAME_MAX - 1] = '\0';
	gethostname(hostname, HOST_NAME_MAX - 1);

	/* Get home directory */
	strcpy(home_dir, passwd->pw_dir);

	/* Get current working directory */
	cwd[PATH_MAX - 1] = '\0';
	getcwd(cwd, PATH_MAX - 1);
}

/** 
 * Function to print and update prompt
 *
 * Parameters: 
 * - void
 *
 * Returns: void
 */
void print_prompt(void) {
	getcwd(cwd, PATH_MAX - 1);

	/* If in home directory, replace with "~" */
	if (startsWith(home_dir, cwd)) {
    	printf("--[%d|%s@%s:%s]--$ ", cmd_id, username, hostname, replace_str(cwd, home_dir, "~"));
	} else {
    	printf("--[%d|%s@%s:%s]--$ ", cmd_id, username, hostname, cwd);
	}

    fflush(stdout);
}

/**
 * Helper function to see if string starts with substring
 *
 * Parameters:
 * - pre: check if str starts with this line
 * - str: check if this line starts with pre
 *
 * Returns: true if successful, false if unsuccessful.
 */
bool startsWith(const char *pre, const char *str) {
	size_t lenpre = strlen(pre),lenstr = strlen(str);
	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

/**
 * Helper function to replace a string with substring
 *
 * Parameters:
 * - str: string to replace
 * - orig: original string
 * - rep: string to replace
 *
 * Returns: new, replaced string
 */
char *replace_str(char *str, char *orig, char *rep) {
	static char buffer[BUF_SZ];
	char *p;
	
	/* Check if 'orig' is in 'str' */
	if (!(p = strstr(str, orig))) {
		return str;
	}

	/* Copy character from 'str' start to 'orig' */
    strncpy(buffer, str, p-str);
	buffer[p-str] = '\0';

	sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

    return buffer;
}

/**
 * Helper function to delete job struct from jobs list
 *
 * Parameters:
 * - pid: pid to delete
 *
 * Returns: void
 */ 
void delete_job(pid_t pid) { 
	/* Search for pid in jobs list */
	int i;
	for (i = 0; i < jobs_i; ++i) {
		if (jobs[i]->pid == pid) {
			break;
		}
	}
	
	/* If pid found in list */
	if (i < jobs_i) {
		/* Reduce size of jobs list */
		jobs_i -= 1;
		/* Move all elements one space ahead */
		for (int j = i; j < jobs_i; ++j) {
			jobs[j] = jobs[j + 1];
		}
	}
}
 
