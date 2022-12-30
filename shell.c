#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct job {
    struct job* prev;
    struct job* next;
    int jobid, length, isBackground;
    pid_t pid;
    char* command[100];
    char* status;
} job;

job* head = NULL;
pid_t fgpid;

void removeJob(job* j) {
    if (j) {
        for (int k = 0; k < j->length; ++k)
            free(j->command[k]);
        if (j->next)
            j->next->prev = j->prev;
        if (j->prev)
            j->prev->next = j->next;
        else
            head = j->next;
        free(j);
        j = NULL;
    }
}

void sigchldHandler(int sig_num) {
    sigset_t mask_all, prev_all;
    sigfillset(&mask_all);
    sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
    job* curr = head;
    for (job* j = head; j; j = curr) {
        curr = j->next;
        if (waitpid(j->pid, NULL, WNOHANG | WUNTRACED) != 0)
            removeJob(j);
    }
    sigprocmask(SIG_SETMASK, &prev_all, NULL);
}

void sigintHandler(int sig) {
    sigset_t mask_all, prev_all;
    sigfillset(&mask_all);
    sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
    for (job* j = head; j; j = j->next) {
        if ((j->pid == fgpid) && waitpid(j->pid, NULL, WNOHANG) == 0) {
            printf("\n[%d] %d terminated by signal 2\n", j->jobid, j->pid);
            kill(j->pid, SIGINT);
            return;
        }
    }
    sigprocmask(SIG_SETMASK, &prev_all, NULL);
}

void sigtstpHandler(int sig_num) {
    sigset_t mask_all, prev_all;
    sigfillset(&mask_all);
    sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
    for (job* j = head; j; j = j->next) {
        printf("hi");
        if ((j->pid == fgpid) && waitpid(j->pid, NULL, WNOHANG) == 0) {
            kill(j->pid, SIGTSTP);
            j->status = "Stopped";
            fgpid = 0;
            printf("\n");
            return;
        }
    }
    sigprocmask(SIG_SETMASK, &prev_all, NULL);
}

void execute(job* j) {
    pid_t pid;
    if (!(pid = fork())) {
        if ((j->command[0][0] == '/' || j->command[0][0] == '.') && execve(j->command[0], j->command, NULL) < 0) {
            printf("%s: No such file or directory\n", j->command[0]);
            exit(1);
        }
        else {
            char dir[100] = "/usr/bin/", oldargv[100];
            strcat(dir, j->command[0]);
            strcpy(oldargv, j->command[0]);
            j->command[0] = dir;
            if (execve(j->command[0], j->command, NULL) < 0) {
                char dir2[100] = "/bin/";
                strcat(dir2, oldargv);
                j->command[0] = dir2;
                if (execve(j->command[0], j->command, NULL) < 0) {
                    printf("%s: command not found\n", oldargv);
                    exit(1);
                }
            }
        }
    }
    else {
        j->pid = pid;
        if (j->isBackground)
            printf("[%d] %d\n", j->jobid, j->pid);
        else {
            fgpid = pid;
            while (waitpid(pid, NULL, WNOHANG) == 0 && waitpid(pid, NULL, WUNTRACED) != pid);
        }
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, sigintHandler);
    signal(SIGTSTP, sigtstpHandler);
    signal(SIGCHLD, sigchldHandler);
    while (!feof(stdin)) {
        char input[1024];
        char* myargv[100];
        int c = 0;
        printf("> ");
        if (!fgets(input, 1023, stdin) || strlen(input) <= 1)
            continue;
        char* token = strtok(input, " \n");
        while (token != NULL) {
            myargv[c++] = token;
            token = strtok(NULL, " \n");
        }
        if (!strcmp(myargv[0], "bg") && myargv[1]) {
            for (job* j = head; j; j = j->next) {
                if (j->jobid == atoi(myargv[1] + 1)) {
                    kill(j->pid, SIGCONT);
                    j->isBackground = 1;
                    j->status = "Running";
                    break;
                }
            }
        }
        else if (!strcmp(myargv[0], "cd") && myargv[1]) {
            char* pwd = (myargv[1]) ? myargv[1] : getenv("HOME");
            chdir(pwd);
            setenv("PWD", pwd, 1);
        }
        else if (!strcmp(myargv[0], "exit")) {
            job* curr = head;
            for (job* j = head; j; j = curr) {
                curr = j->next;
                if (waitpid(j->pid, NULL, WNOHANG) > 0) {
                    kill(j->pid, SIGHUP);
                    kill(j->pid, SIGCONT);
                }
                else if (waitpid(j->pid, NULL, WNOHANG) == 0)
                    kill(j->pid, SIGHUP);
            }
            return EXIT_SUCCESS;
        }
        else if (!strcmp(myargv[0], "fg") && myargv[1]) {
            for (job* j = head; j; j = j->next) {
                if (j->jobid == atoi(myargv[1] + 1)) {
                    pid_t pid = j->pid;
                    if (!(strcmp(j->status, "Stopped")))
                        kill(pid, SIGCONT);
                    j->isBackground = 0;
                    fgpid = j->pid;
                    j->status = "Running";
                    while (waitpid(pid, NULL, WNOHANG) == 0 && waitpid(pid, NULL, WUNTRACED) != pid);
                    break;
                }
            }
        }
        else if (!strcmp(myargv[0], "jobs")) {
            if (head) {
                job* end = head;
                while (end->next)
                    end = end->next;
                for (job* j = end; j; j = j->prev) {
                    printf("[%d] %d %s", j->jobid, j->pid, j->status);
                    for (int k = 0; k < j->length; ++k)
                        printf(" %s", j->command[k]);
                    printf(j->isBackground ? " &\n" : "\n");
                }
            }
        }
        else if (!strcmp(myargv[0], "kill") && myargv[1]) {
            for (job* j = head; j; j = j->next) {
                if (j->jobid == atoi(myargv[1] + 1)) {
                    printf("[%d] %d terminated by signal 15\n", j->jobid, j->pid);
                    kill(j->pid, SIGTERM);
                    removeJob(j);
                    break;
                }
            }
        }
        else {
            job* task = (job*) malloc(sizeof(job) + 100 * 100 * sizeof(char));
            task->prev = NULL;
            task->next = head;
            if (head)
                head->prev = task;
            head = task;
            task->jobid = task->next ? task->next->jobid + 1 : 1;
            for (int i = 0; i < c; ++i) {
                task->command[i] = (char*) malloc(100 * sizeof(char));
                strcpy(task->command[i], myargv[i]);
            }
            if (task->command[c - 1][strlen(task->command[c - 1]) - 1] == '&') {
                if (strlen(task->command[c - 1]) > 1)
                    task->command[c - 1][strlen(task->command[c - 1]) - 1] = '\0';
                else
                    --c;
                task->isBackground = 1;
            }
            else
                task->isBackground = 0;
            task->command[c] = NULL;
            task->length = c;
            task->status = "Running";
            execute(task);
        }
    }
}