#define _GNU_SOURCE
#include "list_rw.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *thread_asc(void *);
void *thread_desc(void *);
void *thread_equal(void *);
void *thread_swap(void *);
void *thread_monitor(void *);

int main() 
{
    Storage *s = storage_init(1000);
    pthread_t t1, t2, t3, t4, t5;
    
    pthread_create(&t1, NULL, thread_asc, s);
    pthread_create(&t2, NULL, thread_desc, s);
    pthread_create(&t3, NULL, thread_equal, s);
    pthread_create(&t4, NULL, thread_swap, s);
    pthread_create(&t5, NULL, thread_monitor, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    pthread_join(t5, NULL);

    storage_destroy(s);
    return 0;
}