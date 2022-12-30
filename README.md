# Shell

## Program Execution

The shell prints a > prompt (greater sign and a space) and reads input from the user. It is able to execute other programs, with command-line options, whether they’re specified with an absolute path:

\> /bin/ls -l

or a relative path:

\> ../src/hello

If the file doesn’t exist, it prints the path and executable, followed by “No such file or directory”. For example,

\> ../src/hello

../src/hello: No such file or directory

If a command is given with no absolute/relative path (e.g., “ls”), and it’s not a built-in command (see below), the shell looks in /usr/bin and /bin (in that order) and executes the first such program found. If it’s not found in either location, the shell prints “commandName: command not found” (with commandName replaced by the actual command). For example,

\> burrito

burrito: command not found

If the command line ends in an ampersand, the shell runs the program in the background. Otherwise, it runs the program in the foreground and waits for the executed program to complete before reading another line of input.

Each line of input defines a job and is assigned a job ID, starting at 1. Job IDs are written with a %, as in %1, to distinguish them from process IDs (PIDs). Jobs also have a status of running, stopped, or terminated.

If a command is run in the background, the shell prints the job ID and process ID, as in:

\> ./hello &

[1] 4061

The command consists of a program and arguments separated by whitespace (possibly leading or trailing). No commands or arguments should have quoted strings or escape characters. For example, these are all valid inputs (with spaces shown explicitly):

␣␣␣./hello␣␣␣␣&

./hello&␣␣␣

./hello␣␣

␣␣␣./hello

␣␣./hello␣␣␣-a␣␣␣-b␣␣␣␣␣␣␣␣-c␣␣␣␣␣␣␣␣&

The shell reaps all zombie children created by any executed process. If any child process terminates due to an unhandled signal, it prints a message with the relevant job ID, process ID, and signal:

[1] 1464 terminated by signal 2

## Signals

- ctrl-c sends SIGINT to the foreground job and any of its child processes

- ctrl-z sends SIGTSTP to the foreground job and any of its child processes

If there is no current foreground job, ctrl-c and ctrl-z do nothing.

SIGINT (by default) will cause the receiving job to terminate. It does not terminate the shell.

SIGTSTP will suspend the receiving job until it receives the SIGCONT signal. It does not suspend the shell. In the shell, you can resume a suspended job in the background (with built-in command bg) or foreground (with built-in command fg), or kill it (with built-in command kill).

## Built-in commands

The shell recognizes these built-in commands:

- bg <jobID>: Run a suspended job in the background.

- cd [path]: Change current directory to the given (absolute or relative) path. If no path is given, the environment variable HOME is used. The shell updates the environment variable PWD with the (absolute) present working directory after running cd.

- exit: Exit the shell. The shell also exits if the user hits ctrl-d on an empty input line. When the shell exits, it first sends SIGHUP followed by SIGCONT to any stopped jobs, and SIGHUP to any running jobs.

- fg <jobID>: Run a suspended or background job in the foreground.

- jobs: List current jobs, including their job ID, process ID, current status, and command. If no jobs exist, this prints nothing.

> jobs

[1] 1432 Running ./test1 &

[2] 1487 Running ./hello &

[3] 1504 Stopped ./foo

Background jobs have an ampersand at the end of the command, whether they were initially run that way or were suspended and then placed in the background.

- kill <jobID>: Send SIGTERM to the given job.

## Examples

To get into the shell, we run

$ ./shell                     # on the normal bash prompt

\>                            # now at your shell prompt, waiting for input

Then we can run a program:

\> /bin/echo hello

hello

\>

We can run something longer and kill it with ctrl-c:

\> /bin/sleep 100

^C

[1] 1464 terminated by signal 2

\>

We can run sleep again and suspend it with ctrl-z:

\> /bin/sleep 100

^Z

\>

Then jobs shows the suspended job:

\> jobs

[1] 1234 Stopped /bin/sleep 100

\>

Then we could foreground it (fg %1), background it (bg %1), or kill it (kill %1).

\> bg %1

\> jobs

[1] 1234 Running /bin/sleep 100 &

\> kill %1

[1] 1234 terminated by signal 15

\> jobs

\>

Running it with & runs it in the background:

\> /bin/sleep 100 &

[1] 1248

\> jobs

[1] 1248 Running /bin/sleep 100 &

\>

If we foreground it, we have to wait for it to complete (or suspend it again):

\> /bin/sleep 100 &

[1] 1248

\> jobs

[1] 1248 Running /bin/sleep 100 &

\> fg %1                      # ...and wait...

\>
