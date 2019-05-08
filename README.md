# Project 2: Command Line Shell

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-2.html

My shell, called crash, is a shell which supports many functions that bash does. It can support comments, background jobs, history, setting evnrionment variables, piping, redirecting, and a lot more. How it works is similar to the bash shell. Upon startup, the shell will prompt the user with their information. The prompt will show the username, and current working directory. When switching directories, the prompt will then show that current directory. After each time the user presses enter, a new line will be outputted, and whatever was in that line will be stored in a database called history. The user can view or access the history with certain commands. This is only a few examples of what crash can do. To build it simply call 'make', then to run call './crash'.
