#define _GNU_SOURCE
#include "list.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *thread_asc(void *);
void *thread_desc(void *);
void *thread_equal(void *);
void *thread_swap_asc(void *);
void *thread_swap_desc(void *);
void *thread_swap_equal(void *);
void *thread_monitor(void *);

int main(int argc, char **argv)
{
    Storage *s = storage_init(1000);
    if (!s) 
    {
        fprintf(stderr, "storage_init failed\n");
        return 1;
    }

    pthread_t t_asc, t_desc, t_equal;
    pthread_t t_sw_asc, t_sw_desc, t_sw_equal;
    pthread_t t_monitor;

    pthread_create(&t_asc,      NULL, thread_asc,       s);
    pthread_create(&t_desc,     NULL, thread_desc,      s);
    pthread_create(&t_equal,    NULL, thread_equal,     s);
    pthread_create(&t_sw_asc,   NULL, thread_swap_asc,  s);
    pthread_create(&t_sw_desc,  NULL, thread_swap_desc, s);
    pthread_create(&t_sw_equal, NULL, thread_swap_equal,s);
    pthread_create(&t_monitor,  NULL, thread_monitor,   NULL);

    pthread_join(t_asc,      NULL);
    pthread_join(t_desc,     NULL);
    pthread_join(t_equal,    NULL);
    pthread_join(t_sw_asc,   NULL);
    pthread_join(t_sw_desc,  NULL);
    pthread_join(t_sw_equal, NULL);
    pthread_join(t_monitor,  NULL);

    storage_destroy(s);
    return 0;
}