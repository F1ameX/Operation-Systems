#define _XOPEN_SOURCE 700
#include "uthread.h"
#include <stdio.h>


static void* worker(void* arg)
{
    printf("Hello from worker %ld\n", (long)arg);
    return NULL;
}


int main (void) 
{
    uthread_t t;
    if (uthread_create(&t, worker, (void*)1) != 0) perror("uthread_create");
    else printf("Thread created successfully\n");
    return 0;
}