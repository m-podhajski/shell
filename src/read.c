#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "config.h"
#include "read.h"


char buf[2 * MAX_LINE_LENGTH];
char buf2[2 * MAX_LINE_LENGTH];
char buf3[2 * MAX_LINE_LENGTH];
int current = 0;
int end = 0;
int n_bytes;

int copy_buffers() {
    int position = -1;
    for (int i = end; i < current; i++) {
        if (buf2[i] == '\n') {
            position = i;
            break;
        }
    }
    if (position != -1) {
        memcpy(buf, buf2, position + 1);
        memmove(buf2, buf2 + position + 1, MAX_LINE_LENGTH - position);
        buf[position + 1] = '\0';
        current = current - (position + 1);
        end = 0;

        return 1;
    }
    return 0;
}

int big_size = 0;
int finished_buffer = 0;

char *read_line2(int *size) {
    while (1) {
        big_size = 0;
        if (finished_buffer) {
            *size = 0;
            return "";
        }
        while (1) {
            if (copy_buffers() == 1)
                break;
            if (current == MAX_LINE_LENGTH) {
                end = 0;
                current -= MAX_LINE_LENGTH;
                big_size = 1;
            }
            if (current != 0) {
                n_bytes = 0;
                int previous = current;
                do {
                    n_bytes = read(STDIN_FILENO, buf2 + current, MAX_LINE_LENGTH - current);
                    current += n_bytes;
                    if (buf2[current - 1] == '\n')
                        break;
                } while (n_bytes && current < MAX_LINE_LENGTH);
                if (!(current - previous) > 0) {
                    finished_buffer = 1;
                    current += 1;
                    buf2[current - 1] = '\n';
                }
            } else {
                n_bytes = 0;
                int previous = 0;
                do {
                    n_bytes = read(STDIN_FILENO, buf2 + current, MAX_LINE_LENGTH - current);
                    current += n_bytes;
                    if (current > 0 && buf2[current - 1] == '\n')
                        break;
                    if (!n_bytes)
                        break;
                } while (current < MAX_LINE_LENGTH);
                if (n_bytes == 0) {
                    finished_buffer = 1;
                    current += 1;
                    buf2[current - 1] = '\n';
                }
            }
        }
        if (big_size) {
            write(2, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));
            write(2, "\n", 1);
        }
        if (!big_size) {
            if (finished_buffer && buf[0] == '\n') {
                *size = 0;
                return "";
            } else {
                if (*size)
                    *size = strlen(buf);
                return buf;
            }
        }
    }
}
