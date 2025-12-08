#define _GNU_SOURCE
#include "list.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void *thread_asc(void *);
void *thread_desc(void *);
void *thread_equal(void *);
void *thread_swap_asc(void *);
void *thread_swap_desc(void *);
void *thread_swap_equal(void *);
void *thread_monitor(void *);

int main(int argc, char **argv)
{
    int n = 1000;

    if (argc > 1) 
    {
        n = atoi(argv[1]);
        if (n <= 0) 
        {
            fprintf(stderr, "Usage: %s [list_size > 0]\n", argv[0]);
            return 1;
        }
    }

    srand((unsigned int)time(NULL));

    Storage *s = storage_init(n);
    if (!s) 
    {
        fprintf(stderr, "storage_init(%d) failed\n", n);
        return 1;
    }

    pthread_t t_asc, t_desc, t_equal;
    pthread_t t_sw_asc, t_sw_desc, t_sw_equal;
    pthread_t t_monitor;

    pthread_create(&t_asc, NULL, thread_asc, s);
    pthread_create(&t_desc, NULL, thread_desc, s);
    pthread_create(&t_equal, NULL, thread_equal, s);

    pthread_create(&t_sw_asc, NULL, thread_swap_asc, s);
    pthread_create(&t_sw_desc, NULL, thread_swap_desc, s);
    pthread_create(&t_sw_equal, NULL, thread_swap_equal, s);

    pthread_create(&t_monitor, NULL, thread_monitor, NULL);
    pthread_join(t_monitor, NULL);

    storage_destroy(s);
    return 0;
}