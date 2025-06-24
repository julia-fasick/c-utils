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
#include <pthread.h> //pthread_cond_t
#endif

#define INITIAL_MAX_QUEUE_LENGTH 32

// PROTOTYPES
int log_exit();
int log_init(int log_level, char* log_path, int delete);
char* msg_formatter(char* level, char* format, va_list args);
char* time_formatter();

int push(char* str);
char* pop(void);
void* writer(void* arg);

// WRITER PROTOTYPES
void fatal(char* format, ...);
void error(char* format, ...);
void warn(char* format, ...);
void info(char* format, ...);
void debug(char* format, ...);
void trace(char* format, ...);

void* writer(void* a);

struct queue_t {
    int terminate;
    pthread_cond_t cond_var;
    pthread_mutex_t mutex;
    char** store;
    size_t max_n_queue;
    size_t n_queue;
    int head;
    int tail;
};

struct queue_t* q;
// GLOBALS
int LOG_LEVEL;
FILE* LOG;
int DELETE;
pthread_t thr;


// TODO: any other params?
// TODO: independent setters for each of these settings post initialization
/**
* Initializes clogger for use.
* @param log_level: The level at which statements should be logged, according to the following standard:
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

    // TODO initialize writer thread
    q = malloc(sizeof(struct queue_t));

    q = calloc(1, sizeof(struct queue_t));
    q->store = calloc(INITIAL_MAX_QUEUE_LENGTH, sizeof(char*));
    q->max_n_queue = INITIAL_MAX_QUEUE_LENGTH;
    q->head = q->tail = q->terminate = 0;

    pthread_cond_init(&q->cond_var, NULL);
    pthread_mutex_init(&q->mutex, NULL);
    pthread_create(&thr, NULL, writer, (void*)q);

    return retcode;
}

int log_exit()
{
    pthread_mutex_lock(&q->mutex);
    q->terminate = 1;
    pthread_cond_signal(&q->cond_var);
    pthread_mutex_unlock(&q->mutex);
    
    pthread_join(thr, NULL);

    for (size_t i = 0; i < q->max_n_queue; ++i) {
        free(q->store[i]);
    }

    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond_var);
    free(q->store);
    free(q);

    if (fclose(LOG) != 0)
    {
        printf("%s\n", "ERROR: Somehow, closing the log file pointer gave an error. Please contact the developer with your usage.");
        return 1;
    }
    return 0;
}

int push(char* str) {
    // first lock mutex
    pthread_mutex_lock(&q->mutex);
    if (q->n_queue == q->max_n_queue) {
        // queue full, realloc
        size_t new_capacity = q->max_n_queue * 2;
        char** new_store = calloc(new_capacity, sizeof(char*));

        if (q->head <  q->tail) {
            // if head is before tail (ie, tail has not wrapped around), we can just add new elements without recalculating anything.
            memcpy(new_store, &q->store[q->head], q->n_queue * sizeof(char*));
        } else {
            // if tail has wrapped around (ie, tail is before head in memory), we must add new elements after head, then shift head and the elements after it by x elements.
            // copy from head to end of current queue
            memcpy(new_store, &q->store[q->head], (q->max_n_queue - q->head) * sizeof(char*));
            // copy from end of current queue to tail - 1
            memcpy(&new_store[(q->max_n_queue - q->head)], &q->store[0], q->tail * sizeof(char*));
        }

        free(q->store);
        q->store = new_store;
        q->head = 0;
        q->tail = q->n_queue;
        q->max_n_queue = new_capacity;
    }

    // add str to queue, add to count, and signal with new_msg flag
    q->store[q->tail] = strdup(str);
    q->tail = (q->tail + 1) % q->max_n_queue;
    q->n_queue++;

    // unlock mutex and signal if thread is waiting
    pthread_mutex_unlock(&q->mutex);
    pthread_cond_signal(&q->cond_var);
    return 0;
}

char* pop(void) {
    if (q->n_queue == 0) {
        return NULL;
    }
    char* result = q->store[q->head];
    q->store[q->head] = NULL; // clear slot
    q->head = (q->head + 1) % q->max_n_queue;
    q->n_queue--;
    return result;
}

void* writer(void *arg)
{
    struct queue_t* q = (struct queue_t*)arg;
    while(1) {
        pthread_mutex_lock(&q->mutex);
        while (q->n_queue == 0 && !q->terminate) {
            pthread_cond_wait(&q->cond_var, &q->mutex);
        }

        if (q->n_queue == 0 && q->terminate) {
            pthread_mutex_unlock(&q->mutex);
            break;
        }

        char* msg = pop();
        pthread_mutex_unlock(&q->mutex);

        if (msg) {
            fwrite(msg, sizeof(char), strlen(msg), LOG);
            free(msg);
        }
    }
    return NULL;
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
    push(res);
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
    push(res);
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
    push(res);
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
    push(res);
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
    push(res);
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
    push(res);
    return;
}