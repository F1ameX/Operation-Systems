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
        int count = 0;

        while (cur && cur->next) 
        {
            pthread_spin_lock(&cur->sync);
            pthread_spin_lock(&cur->next->sync);

            int l1 = strlen(cur->value);
            int l2 = strlen(cur->next->value);
            if (l1 < l2) count++;

            pthread_spin_unlock(&cur->next->sync);
            pthread_spin_unlock(&cur->sync);

            cur = cur->next;
        }

        asc_iterations++;
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
            pthread_spin_lock(&cur->sync);
            pthread_spin_lock(&cur->next->sync);

            int l1 = strlen(cur->value);
            int l2 = strlen(cur->next->value);
            if (l1 > l2) swaps_done++;

            pthread_spin_unlock(&cur->next->sync);
            pthread_spin_unlock(&cur->sync);

            cur = cur->next;
        }

        desc_iterations++;
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
            pthread_spin_lock(&cur->sync);
            pthread_spin_lock(&cur->next->sync);

            int l1 = strlen(cur->value);
            int l2 = strlen(cur->next->value);
            if (l1 == l2) equal_iterations++;

            pthread_spin_unlock(&cur->next->sync);
            pthread_spin_unlock(&cur->sync);

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

        if (prev) pthread_spin_lock(&prev->sync);
        pthread_spin_lock(&a->sync);
        pthread_spin_lock(&b->sync);

        a->next = b->next;
        b->next = a;
        if (prev) prev->next = b;
        else s->first = b;

        pthread_spin_unlock(&b->sync);
        pthread_spin_unlock(&a->sync);
        if (prev) pthread_spin_unlock(&prev->sync);
    }

    return NULL;
}

void *thread_monitor(void *arg) 
{
    (void)arg;
    
    while (1) 
    {
        printf("[monitor][spin] asc=%ld desc=%ld equal=%ld swaps=%ld\n", asc_iterations, desc_iterations, equal_iterations, swaps_done);
        fflush(stdout);
        sleep(2);
    }
    return NULL;
}