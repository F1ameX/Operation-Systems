#define _GNU_SOURCE
#include "list.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static long asc_iterations = 0;
static long desc_iterations = 0;
static long equal_iterations = 0;
static long swaps_done = 0;

void *thread_asc(void *arg) 
{
    Storage *s = (Storage *)arg;
    while (1) 
    {
        Node *cur = s->first;

        while (cur && cur->next) 
        {
            pthread_rwlock_rdlock(&cur->sync);
            pthread_rwlock_rdlock(&cur->next->sync);

            int l1 = strlen(cur->value);
            int l2 = strlen(cur->next->value);
            if (l1 < l2) asc_iterations++;

            pthread_rwlock_unlock(&cur->next->sync);
            pthread_rwlock_unlock(&cur->sync);

            cur = cur->next;
        }
    }
    return NULL;
}

void *thread_desc(void *arg) 
{
    Storage *s = (Storage *)arg;

    while (1) 
    {
        Node *cur = s->first;

        while (cur && cur->next) 
        {
            pthread_rwlock_rdlock(&cur->sync);
            pthread_rwlock_rdlock(&cur->next->sync);

            int l1 = strlen(cur->value);
            int l2 = strlen(cur->next->value);
            if (l1 > l2) desc_iterations++;

            pthread_rwlock_unlock(&cur->next->sync);
            pthread_rwlock_unlock(&cur->sync);

            cur = cur->next;
        }
    }
    return NULL;
}

void *thread_equal(void *arg) 
{
    Storage *s = (Storage *)arg;

    while (1) 
    {
        Node *cur = s->first;

        while (cur && cur->next) 
        {
            pthread_rwlock_rdlock(&cur->sync);
            pthread_rwlock_rdlock(&cur->next->sync);

            int l1 = strlen(cur->value);
            int l2 = strlen(cur->next->value);
            if (l1 == l2) equal_iterations++;

            pthread_rwlock_unlock(&cur->next->sync);
            pthread_rwlock_unlock(&cur->sync);

            cur = cur->next;
        }
    }
    return NULL;
}

void *thread_swap(void *arg) 
{
    Storage *s = (Storage *)arg;
    srand(time(NULL) ^ getpid());

    while (1) 
    {
        int idx = rand() % (s->count - 2);
        Node *prev = NULL;
        Node *a = s->first;

        for (int i = 0; i < idx && a && a->next; i++) 
        {
            prev = a;
            a = a->next;
        }

        if (!a || !a->next) continue;
        Node *b = a->next;

        if (prev) pthread_rwlock_wrlock(&prev->sync);
        pthread_rwlock_wrlock(&a->sync);
        pthread_rwlock_wrlock(&b->sync);

        a->next = b->next;
        b->next = a;
        if (prev) prev->next = b;
        else s->first = b;

        swaps_done++;

        pthread_rwlock_unlock(&b->sync);
        pthread_rwlock_unlock(&a->sync);
        if (prev) pthread_rwlock_unlock(&prev->sync);
    }

    return NULL;
}

void *thread_monitor(void *arg) 
{
    (void)arg;
    
    while (1) 
    {
        printf("[monitor][rwlock] asc=%ld desc=%ld equal=%ld swaps=%ld\n", asc_iterations, desc_iterations, equal_iterations, swaps_done);
        fflush(stdout);
        sleep(2);
    }
    return NULL;
}