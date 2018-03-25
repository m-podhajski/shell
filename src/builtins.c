#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>

#include "config.h"
#include "errors.h"
#include "builtins.h"

int echo(char *[]);

int lexit(char *argv[]);

int undefined(char *[]);

int lcd(char *argv[]);

int lkill(char *argv[]);

int lls(char *argv[]);

builtin_pair builtins_table[] = {
        {"exit",  &lexit},
        {"lecho", &echo},
        {"lcd",   &lcd},
        {"lkill", &lkill},
        {"lls",   &lls},
        {NULL,    NULL}
};

int
echo(char *argv[]) {
    int i = 1;
    if (argv[i]) printf("%s", argv[i++]);
    while (argv[i])
        printf(" %s", argv[i++]);

    printf("\n");
    fflush(stdout);
    return 0;
}

int lkill(char *argv[]) {
    int signal_number;
    int pid;
    if (argv[1] == NULL)
        return -1;
    if (argv[1][0] == '-') {
        signal_number = atoi(argv[1] + 1);
        pid = atoi(argv[2]);
    } else {
        signal_number = SIGTERM;
        pid = atoi(argv[1]);
    }
    return kill(pid, signal_number);
}

int lls(char *argv[]) {
    char current_path[2 * MAX_LINE_LENGTH];
    char *path;
    if (argv[1] == NULL) {
        if (getcwd(current_path, MAX_LINE_LENGTH) == NULL)
            return -1;
        path = current_path;
    } else {
        path = argv[1];
    }
    DIR *directory = opendir(path);
    if (directory == NULL)
        return -1;
    struct dirent *dp;
    while ((dp = readdir(directory)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;
        printf("%s\n", dp->d_name);
        fflush(stdout);
    }
    closedir(directory);
    return 0;
}

int lexit(char *argv[]) {
    exit(0);
    return 0;
}

int lcd(char *argv[]) {
    char *path;
    if (argv[2])
        return -1;
    if (argv[1])
        path = argv[1];
    else
        path = getenv("HOME");
    int res = chdir(path);
    return res;
}

int
undefined(char *argv[]) {
    fprintf(stderr, "Command %s undefined.\n", argv[0]);
    return BUILTIN_ERROR;
}

int check_builtins(char *in) {
    int x;
    char comp[MAX_LINE_LENGTH];
    for (int i = 0; in[i] != ' ' && in[i] != '\0' && in[i] != '\n'; i++) {
        comp[i] = in[i];
        x = i;
    }
    comp[x + 1] = '\0';
    for (int i = 0; builtins_table[i].name != NULL; i++) {
        if (strcmp(builtins_table[i].name, comp) == 0) {
            return 1;
        }
    }
    return 0;
}

int builtins(command *in) {
    for (int i = 0; builtins_table[i].name != NULL; i++) {
        if (strcmp(builtins_table[i].name, in->argv[0]) == 0) {
            if (builtins_table[i].fun(in->argv) == -1) {
                write(2, "Builtin ", 8);
                write(2, in->argv[0], strlen(in->argv[0]));
                write(2, " error.\n", 8);
                return -1;
            }
            return 0;
        }
    }
    return 1;
}
