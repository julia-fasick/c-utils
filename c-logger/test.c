#include "clogger.h"

#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[])
    {
        if (log_init(6, "./log.txt", 0) != 0) {
            printf("%s\n", "From test.c: initialization error!");
        }
        clock_t begin = clock();
        for(int i = 0; i < 50000; i++) {
            fatal("from fatal %s%d\n", "blah blah ", i);
            error("from error %s%d\n", "blah blah ", i);
            warn("from warn %s%d\n", "blah blah ", i);
            info("from info %s%d\n", "blah blah ", i);
            debug("from debug %s%d\n", "blah blah ", i);
            trace("from trace %s%d\n", "blah blah ", i);
        }
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        if (log_exit() != 0) {
            printf("%s\n", "From test.c: exit error!");
        }
        printf("Time Spent Without Threading: %.6g seconds\n", time_spent);
    }