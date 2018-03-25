#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "siparse.h"
#include "utils.h"
#include "config.h"
#include "errors.h"
#include "builtins.h"
#include "sig.h"

line *ln;
command *com;
int *first_pipe = NULL;
int *second_pipe = NULL;

void redirect(command *com) {
    int i = 0;
    while (com->redirs[i] != NULL) {
        if (IS_RIN(com->redirs[i]->flags)) {
            int x = open(com->redirs[i]->filename, O_RDONLY);
            errors_redirect(x, com->redirs[i]->filename);
            dup2(x, STDIN_FILENO);
            close(x);
        }
        if (IS_ROUT(com->redirs[i]->flags)) {
            int x = open(com->redirs[i]->filename, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            errors_redirect(x, com->redirs[i]->filename);
            dup2(x, STDOUT_FILENO);
            close(x);
        }
        if (IS_RAPPEND(com->redirs[i]->flags)) {
            int x = open(com->redirs[i]->filename, O_APPEND | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            errors_redirect(x, com->redirs[i]->filename);
            dup2(x, STDOUT_FILENO);
            close(x);
        }
        i++;
    }
}

void check_line(char *bufor, int size) {
//if (size <= MAX_LINE_LENGTH)
    ln = parseline(bufor);
    if (!ln || size > MAX_LINE_LENGTH) {
        write(2, SYNTAX_ERROR_STR, 13);
        write(2, "\n", 1);
    }
}

void exec_command(command *com) {
    if (com == NULL) return;
    if (com->argv == NULL) return;
    int ch = fork();
    if (ch == 0) {
        if (background())
            setsid();
        sigaction(SIGINT, &sig1, NULL);
        sigaction(SIGCHLD, &sig2, NULL);
        if (first_pipe != NULL) {
            dup2(first_pipe[0], STDIN_FILENO);
            close(first_pipe[0]);
            close(first_pipe[1]);
        }
        if (second_pipe != NULL) {
            dup2(second_pipe[1], STDOUT_FILENO);
            close(second_pipe[0]);
            close(second_pipe[1]);
        }
        redirect(com);
        int result = execvp(com->argv[0], com->argv);
        errors(result, com, com->argv[0]);
        exit(0);
    } else if (ch > 0) {
        if (!background())
            add_foreground_process(ch);
    } else {
        exit(1);
    }
}

void exec_pipeline(pipeline pipeline_) {
    sigchld_block_();
    first_pipe = NULL;
    second_pipe = NULL;
    set_size(0);
    set_counter(0);
    for (int i = 0; pipeline_[i] != NULL; i++) {
        second_pipe = NULL;
        if (pipeline_[i + 1] != NULL) {
            second_pipe = malloc(2);
            pipe(second_pipe);
        }
        if (pipeline_[i]->argv[0] != NULL)
            exec_command(pipeline_[i]);
        if (first_pipe) {
            close(first_pipe[0]);
            close(first_pipe[1]);
            free(first_pipe);
        }
        first_pipe = second_pipe;
    }
    if (first_pipe) {
        close(first_pipe[0]);
        close(first_pipe[1]);
        free(first_pipe);
    }
    while (!background() && foreground_sze() > 0) {
        sigsuspend(&wait_);
    }
    sigchld_unblock();
}

void execute(char *bufor, int size) {
    if (bufor[size - 1] == '\n')
        bufor[size - 1] = '\0';
    if (check_builtins(bufor) == 1) {
        check_line(bufor, size);
        com = pickfirstcommand(ln);
        builtins(com);
        return;
    }
    if (size <= MAX_LINE_LENGTH)
        ln = parseline(bufor);
    if (!ln || size > MAX_LINE_LENGTH) {
        write(2, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));
        write(2, "\n", 1);
        exit(1);
    }
    set_background(ln->flags);
    for (int i = 0; ln->pipelines[i] != NULL; i++) {
        pipeline pipeline_ = ln->pipelines[i];
        exec_pipeline(pipeline_);
    }
}

