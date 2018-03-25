#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "sig.h"
#include "config.h"
#include "siparse.h"
#include "utils.h"
#include "errors.h"
#include "builtins.h"
#include "read.h"
#include "execute.h"


int
main(int argc, char *argv[]) {
    sigprocmask(SIG_BLOCK, NULL, &wait_);
    set_handlers();
    while (1) {
        prompt();
        int *size;
        char *in = read_line2(size);
        if (!*size)
            break;
        execute(in, *size);
    }
    return 0;
}
