#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "siparse.h"
#include "config.h"
#include "utils.h"
#include "errors.h"


void errors(int in, command *com, char *bufor) {
    if (in != -1)
        return;
    switch (errno) {
        case (ENOENT): {
            write(2, com->argv[0], strlen(com->argv[0]));
            write(2, ": no such file or directory\n", 28);
            break;
        }
        case (EACCES): {
            write(2, bufor, strlen(bufor));
            write(2, ": permission denied\n", 20);
            break;
        }
        default: {
            write(2, bufor, strlen(bufor));
            write(2, ":exec error\n", 13);
            break;
        }
    }
    exit(EXEC_FAILURE);
}

void errors_redirect(int in, char *file) {
    if (in != -1)
        return;
    switch (errno) {
        case (ENOENT): {
            write(2, file, strlen(file));
            write(2, ": no such file or directory\n", 28);
            break;
        }
        case (EACCES): {
            write(2, file, strlen(file));
            write(2, ": permission denied\n", 20);
            break;
        }
    }
    exit(EXEC_FAILURE);
}
