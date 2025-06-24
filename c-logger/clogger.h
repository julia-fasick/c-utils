// TODO: check if this is up to convention
// TODO: organize includes
// TODO: alphabatize prototypes, globals, defs, etc.
#ifndef _STD_INCLUDES
#include <stdarg.h> // va_args
#include <stddef.h> // NULL
#include <stdio.h> // vsprintf, fopen, FILE
#include <time.h> // time, localtime
#include <stdlib.h> // malloc
#include <unistd.h> // access
#include <limits.h> // PATH_MAX
#include <string.h> // strlen
#endif

// PROTOTYPES
int log_exit();
int log_init(int log_level, char* log_path, int delete);
char* msg_formatter(char* level, char* format, va_list args);
char* time_formatter();

// WRITER PROTOTYPES
void fatal(char* format, ...);
void error(char* format, ...);
void warn(char* format, ...);
void info(char* format, ...);
void debug(char* format, ...);
void trace(char* format, ...);

// GLOBALS
int LOG_LEVEL;
FILE* LOG;
int DELETE;


// DEFINITIONS

int log_exit()
{
    if (fclose(LOG) != 0)
    {
        printf("%s\n", "ERROR: Somehow, closing the log file pointer gave an error. Please contact the developer with your usage.");
        return 1;
    }
    return 0;
}

// TODO: any other params?
// TODO: independent setters for each of these settings post initialization
/**
* Initializes clogger for use.
*   @param log_level: The level at which statements should be logged, according to the following standard:
*       1: FATAL
*       2: ERROR
*       3: WARN
*       4: INFO
*       5: DEBUG
*       6: TRACE
* @param log_path: The path to use (can be non-existant).
* @param delete: Whether or not to delete the contents of the file pointed to by log_path, if it exists.
*/
int log_init(int log_level, char* log_path, int delete)
{
    // some errors, like incorrect log_level/delete setting, we can correct automagically and not exit. we will use this retcode to keep track of whether the function
    // runs without an error.
    int retcode = 0;

    // establish globals. note that we cant establish LOG_PATH yet as we have to resolve and possible create it.
    // log_level check
    if (log_level < 1 || log_level > 6)
    {
        printf("ERROR: log_level given in init() call was %d. log_level should be set according to this standard:\n\t1:\tFATAL\n\t2:\tERROR\n\t3:\tWARN\n\t4:\tINFO\n\t5:\tDEBUG\n\t6:\tTRACE\nDefaulting to TRACE (6)...\n", log_level);
        LOG_LEVEL = 6;
        retcode = 1;
    } else
    {
        LOG_LEVEL = log_level;
    }
    
    // delete check
    if (delete != 0 && delete != 1)
    {
        printf("ERROR: delete given in init() call was %d. delete should be set according to this standard:\n\t0:\tTRUE\n\t1:\tFALSE\nDefaulting to FALSE (1)...\n", delete);
        DELETE = 1;
        retcode = 1;
    } else
    {
        DELETE = delete;
    }

    // check if log already exists at path
    // if DELETE is 1, we want to open with flag "w" as that will create if OR open and truncate to length 0, dependent on if it exists
    if (DELETE == 1)
    {
        LOG = fopen(log_path, "w");
    } else {
        LOG = fopen(log_path, "a");
    }
    return retcode;
}

/**
* Returns a memory-allocated formatted character string according to the format provided.
* Don't forget to free the returned char pointer!
* @param format: string containing the format string.
* @param ...: variable amount of arguments according to how many format specifiers are included in the format string.
 */
char* msg_formatter(char* level, char* format, va_list args)
{
    // get time
    char* time_buf = time_formatter();
    // format message
    char* msg_buf = malloc(sizeof(char) * 1024);
    vsprintf(msg_buf, format, args);
    char* res = malloc(sizeof(char) * 2048);
    sprintf(res, "%s - %s - %s", time_buf, level, msg_buf);
    free(time_buf);
    free(msg_buf);
    return res;
}

char* time_formatter()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char* time_buf = malloc(sizeof(char) * 1024);
    sprintf(time_buf, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return time_buf;
}


void fatal(char* format, ...)
{
    if (LOG_LEVEL < 1)
    {
        return;
    }
    // init va_args
    va_list args;
    va_start(args, format);
    char* res = msg_formatter("FATAL", format, args);
    va_end(args);
    // TODO: check write success using size_t return value of fwrite and compare that to strlen(res), if equal then write success
    fwrite(res, sizeof(char), strlen(res), LOG);
    free(res);
    return;
}

void error(char* format, ...)
{
    if (LOG_LEVEL < 2)
    {
        return;
    }
    // init va_args
    va_list args;
    va_start(args, format);
    char* res = msg_formatter("ERROR", format, args);
    va_end(args);
    // TODO: check write success using size_t return value of fwrite and compare that to strlen(res), if equal then write success
    fwrite(res, sizeof(char), strlen(res), LOG);
    free(res);
    return;
}

void warn(char* format, ...)
{
    if (LOG_LEVEL < 3)
    {
        return;
    }
    // init va_args
    va_list args;
    va_start(args, format);
    char* res = msg_formatter("WARN", format, args);
    va_end(args);
    // TODO: check write success using size_t return value of fwrite and compare that to strlen(res), if equal then write success
    fwrite(res, sizeof(char), strlen(res), LOG);
    free(res);
    return;
}

void info(char* format, ...)
{
    if (LOG_LEVEL < 4)
    {
        return;
    }
    // init va_args
    va_list args;
    va_start(args, format);
    char* res = msg_formatter("INFO", format, args);
    va_end(args);
    // TODO: check write success using size_t return value of fwrite and compare that to strlen(res), if equal then write success
    fwrite(res, sizeof(char), strlen(res), LOG);
    free(res);
    return;
}
void debug(char* format, ...)
{
    if (LOG_LEVEL < 5)
    {
        return;
    }
    // init va_args
    va_list args;
    va_start(args, format);
    char* res = msg_formatter("DEBUG", format, args);
    va_end(args);
    // TODO: check write success using size_t return value of fwrite and compare that to strlen(res), if equal then write success
    fwrite(res, sizeof(char), strlen(res), LOG);
    free(res);
    return;
}
void trace(char* format, ...)
{
    if (LOG_LEVEL < 6)
    {
        return;
    }
    // init va_args
    va_list args;
    va_start(args, format);
    char* res = msg_formatter("TRACE", format, args);
    va_end(args);
    // TODO: check write success using size_t return value of fwrite and compare that to strlen(res), if equal then write success
    fwrite(res, sizeof(char), strlen(res), LOG);
    free(res);
    return;
}
