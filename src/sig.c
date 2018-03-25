#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "config.h"
#include "siparse.h"
#include "utils.h"
#include "errors.h"
#include "builtins.h"
#include "sig.h"

#define SIZE 100


struct sigaction sig1;
struct sigaction sig2;
struct sigaction siginterupt;
struct sigaction sigchld;

int fg_pids[SIZE];
int fg_sz = 0;
int fg_size = 0;
int bg_pids[SIZE];
int bg_stats[SIZE];

int counter = 0;

int background_process;

sigset_t wait_;
sigset_t sigset;


void sigchld_handler() {
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    while (pid > 0) {
        int flag = 1;
        for (int i = 0; i < fg_size; i++) {
            if (pid == fg_pids[i]) {
                fg_sz--;
                flag = 0;
                break;
            }
        }
        if (counter < SIZE && flag) {
            bg_pids[counter] = pid;
            bg_stats[counter] = status;
            counter++;
        }
        pid = waitpid(-1, &status, WNOHANG);
    }
}

void add_foreground_process(int in) {
    fg_sz++;
    fg_pids[fg_size] = in;
    fg_size++;
}

void sigchld_block_() {
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
}

void sigchld_unblock() {
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}

void set_handlers() {
    siginterupt.sa_handler = SIG_IGN;
    sigchld.sa_handler = sigchld_handler;
    sigaction(SIGINT, &siginterupt, &sig1);
    sigaction(SIGCHLD, &sigchld, &sig2);
}

void prompt() {
    struct stat buffer;
    fstat(0, &buffer);
    int nbytes = strlen(PROMPT_STR);
    if (S_ISCHR(buffer.st_mode)) {
        int i = 0;
        for (i = 0; i < counter; i++) {
            int stat = bg_stats[i];
            int pid = bg_pids[i];
            if (WIFEXITED(stat)) {
                int x = WEXITSTATUS(stat);
                printf("Background process %i terminated. (exited with status %i)\n", pid, x);
            } else {
                int x = WTERMSIG(stat);
                printf("Background process %i terminated. (killed by signal %i)\n", pid, x);
            }
        }
        counter = 0;
        write(1, PROMPT_STR, nbytes);
        fflush(stdout);
    }
}

int background() {
    return background_process == LINBACKGROUND;
}

int bckgrnd_proc() {
    return background_process;
}

void set_background(int in) {
    background_process = in;
}

void set_size(int in) {
    fg_size = in;
}

void set_counter(int in) {
    fg_sz = in;
}

int foreground_sze() { return fg_sz; }
