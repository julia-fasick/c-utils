# c-utils

A (currently quite small) collection of utilities for C programming.

### c-logger

c-logger is a simple logging utility. There are two versions. `clogger.h` is a lightweight version for single-threaded programs, and `cloggerwiththreads` is a slightly more robust version designed for use in a multi-threaded environments. It uses a thread safe, dynamically allocated FIFO queue to ensure synchronization. Read it's respective README for more information.