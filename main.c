#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "threadpool/threadpool.h"

static void demoTask(void *arg) 
{
    long id = *(long *)arg;
    free(arg);

    printf("[task %ld] started\n", id);
    usleep(100 * 1000);
    printf("[task %ld] finished\n", id);
}

int main(void) 
{
    threadPool_t *pool = threadPoolCreate(3);
    if (!pool) 
    {
        fprintf(stderr, "failed to create thread pool\n");
        return 1;
    }

    for (long i = 0; i < 8; ++i) 
    {
        long *id = malloc(sizeof(long));
        if (!id) 
        {
            fprintf(stderr, "malloc failed\n");
            break;
        }
        *id = i;
        if (threadPoolSubmit(pool, demoTask, id) != 0) 
        {
            fprintf(stderr, "submit failed for task %ld\n", i);
            free(id);
        }
    }

    sleep(1);

    threadPoolStop(pool);
    return 0;
}