#define _XOPEN_SOURCE 700
#include "uthread.h"
#include <stdio.h>

static void* worker(void* arg)
{
    long id = (long)arg;
    for (int i = 0; i < 3; ++i) {
        printf("[uthread %ld] iter %d\n", id, i);
        uthread_yield();
    }
    return NULL;
}

int main(void)
{
    uthread_t a, b, c;

    if (uthread_create(&a, worker, (void*)1) || uthread_create(&b, worker, (void*)2) || uthread_create(&c, worker, (void*)3)) 
    {
        perror("uthread_create");
        return 1;
    }

    uthread_run_all();
    return 0;
}