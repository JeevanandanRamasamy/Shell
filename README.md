# Shell

## Program Execution

The shell prompts the user with a `>` sign and reads input. It is capable of executing programs using either an absolute or relative path, as shown below:

```bash
> /bin/ls -l
```

or

```bash
> ../src/hello
```

If a file doesn’t exist, the shell outputs the path and executable followed by “No such file or directory”:

```bash
> ../src/hello
../src/hello: No such file or directory
```

If no absolute/relative path is provided for a command (e.g., “ls”), and it’s not a built-in command, the shell will search in `/usr/bin` and `/bin` (in that order) and execute the first matching program found. If it’s not found in either directory, the shell prints an error message:

```
> burrito
burrito: command not found
```

If a command ends with an ampersand (`&`), the program is executed in the background. Otherwise, it runs in the foreground, and the shell waits for the program to complete before prompting for the next input.

Each line of input represents a job and is assigned a job ID starting from 1. Job IDs are shown with a `%`, e.g., `%1`, to distinguish them from process IDs (PIDs). Jobs can have one of the following statuses: running, stopped, or terminated.

For background jobs, the shell displays the job ID along with the process ID:

```
> ./hello &
[1] 4061
```

The command consists of a program and its arguments, separated by whitespace (spaces or tabs). No commands or arguments should include quoted strings or escape characters. These are all valid inputs (with spaces shown explicitly):

```
   ./hello   
./hello&   
./hello   
   ./hello   
   ./hello -a -b -c &
```

The shell reaps all zombie children created by executed processes. If a child process terminates due to an unhandled signal, the shell prints a message with the relevant job ID, process ID, and signal:

```
[1] 1464 terminated by signal 2
```

## Signals

- `ctrl-c` sends **SIGINT** to the foreground job and any of its child processes.
- `ctrl-z` sends **SIGTSTP** to the foreground job and any of its child processes.

If there is no current foreground job, `ctrl-c` and `ctrl-z` will do nothing.

- **SIGINT** (by default) will cause the receiving job to terminate. It does not terminate the shell.
- **SIGTSTP** suspends the receiving job until it receives the **SIGCONT** signal. It does not suspend the shell. In the shell, you can resume a suspended job in the background (with the built-in command `bg`), foreground (with the built-in command `fg`), or kill it (with the built-in command `kill`).

## Built-in Commands

The shell supports the following built-in commands:

- `bg <jobID>`: Run a suspended job in the background.
- `cd [path]`: Change the current directory to the given (absolute or relative) path. If no path is provided, the shell uses the environment variable `HOME`. After running `cd`, the shell updates the `PWD` variable with the current working directory.
- `exit`: Exit the shell. The shell also exits if the user presses `ctrl-d` on an empty input line. When the shell exits, it sends **SIGHUP** followed by **SIGCONT** to any stopped jobs, and **SIGHUP** to any running jobs.
- `fg <jobID>`: Run a suspended or background job in the foreground.
- `jobs`: List the current jobs, including their job ID, process ID, status, and command. If no jobs exist, this prints nothing.

```
> jobs
[1] 1432 Running ./test1 &
[2] 1487 Running ./hello &
[3] 1504 Stopped ./foo
```

- `kill <jobID>`: Send **SIGTERM** to the specified job.

## Examples

1. **Starting the Shell**

To enter the shell, run:

```
$ ./shell                # on the normal bash prompt
>                         # now at your shell prompt, waiting for input
```

2. **Running a Program**

Run a program like `echo`:

```
> /bin/echo hello
hello
>
```

3. **Running and Killing a Process**

Run a process (e.g., `sleep`) and then kill it using `ctrl-c`:

```
> /bin/sleep 100
^C
[1] 1464 terminated by signal 2
>
```

4. **Suspending a Process**

Run `sleep` and suspend it with `ctrl-z`:

```
> /bin/sleep 100
^Z
>
> jobs
[1] 1234 Stopped /bin/sleep 100
```

Then you can either foreground it (`fg %1`), background it (`bg %1`), or kill it (`kill %1`):

```
> bg %1
> jobs
[1] 1234 Running /bin/sleep 100 &
> kill %1
[1] 1234 terminated by signal 15
> jobs
>
```

5. **Running in the Background**

Run `sleep` in the background:

```
> /bin/sleep 100 &
[1] 1248
> jobs
[1] 1248 Running /bin/sleep 100 &
>
```

6. **Foregrounding a Process**

If you run a process in the background, you can bring it to the foreground with `fg`:

```
> /bin/sleep 100 &
[1] 1248
> jobs
[1] 1248 Running /bin/sleep 100 &
> fg %1        # ...and wait...
>
```
