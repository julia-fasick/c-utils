# c-logger

c-logger is a simple logging utility. There are two versions. `clogger.h` is a leightweight version for single-threaded programs, and `cloggerwiththreads.h` is a slightly more robust version designed for use in multi-threaded environments. It uses a thread safe, dynamically allocated FIFO queue to ensure proper synchronization.

## Usage

Simply put the header in your working directory, add `#include "clogger.h"` to your includes, and call log_init(LOG_LEVEL, LOG_PATH, DELETE), using values from below.

LOG_LEVEL can be:
1. FATAL
2. ERROR
3. WARN
4. INFO
5. DEBUG
6. TRACE

LOG_PATH can be non-existant or already created.

DELETE can be 0 for false or 1 for true. With true, the file will be completely emptied before any new data is written. With false, the new log will be appended to the bottom of the file.

Then, you can use the variadic functions provided for each log level to write formatted messages to log. See the example below from the test.c file.

Make sure to call log_exit before your program terminates! Failure to do so could cause memory leaks or a zombie process.

## Notes

The muti-threaded header file provides significantly worse overall performance due to context switching and dynamic allocation. When tested with the simple script below, we see that the multi-threaded version takes about 2x as long as the single-threaded version (1.5s vs 3s on my machine).


    int main(int argc, char *argv[])
    {
        log_init(6, "./log.txt", 0);
        clock_t begin = clock();
        for (int i = 0; i < 50000; i++)
        {
            fatal("from fatal %s%d\n", "blah blah ", i);
            error("from error %s%d\n", "blah blah ", i);
            warn("from warn %s%d\n", "blah blah ", i);
            info("from info %s%d\n", "blah blah ", i);
            debug("from debug %s%d\n", "blah blah ", i);
            trace("from trace %s%d\n", "blah blah ", i);
        }
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        log_exit();
        printf("Time Spent Without Threading: %.6g seconds\n", time_spent);
    }
