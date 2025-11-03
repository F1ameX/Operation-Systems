#define _GNU_SOURCE
#include "list.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *thread_asc(void *);
void *thread_desc(void *);
void *thread_equal(void *);
void *thread_swap(void *);
void *thread_monitor(void *);

int main() 
{
    Storage *s = storage_init(1000); 
    pthread_t t_asc, t_desc, t_equal, t_swap, t_monitor;

    pthread_create(&t_asc, NULL, thread_asc, s);
    pthread_create(&t_desc, NULL, thread_desc, s);
    pthread_create(&t_equal, NULL, thread_equal, s);
    pthread_create(&t_swap, NULL, thread_swap, s);
    pthread_create(&t_monitor, NULL, thread_monitor, NULL);

    pthread_join(t_asc, NULL);
    pthread_join(t_desc, NULL);
    pthread_join(t_equal, NULL);
    pthread_join(t_swap, NULL);
    pthread_join(t_monitor, NULL);

    storage_destroy(s);
    return 0;
}