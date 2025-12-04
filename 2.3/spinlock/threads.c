#define _GNU_SOURCE
#include "list.h"
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static long asc_iterations   = 0;
static long desc_iterations  = 0;
static long equal_iterations = 0;

static long asc_pairs = 0; // пары с l1 <  l2
static long desc_pairs = 0; // пары с l1 >  l2
static long equal_pairs = 0; // пары с l1 == l2

static long asc_swaps = 0; // свопы, исправляющие "не-возрастание"
static long desc_swaps = 0; // свопы, исправляющие "не-убывание"
static long equal_swaps = 0; // свопы, исправляющие "не-равенство"

static pthread_mutex_t counters_lock = PTHREAD_MUTEX_INITIALIZER;

static void inc_long(long *p)
{
    pthread_mutex_lock(&counters_lock);
    ++(*p);
    pthread_mutex_unlock(&counters_lock);
}

static void snapshot_counters(long *ai, long *di, long *ei, long *as, long *ds, long *es)
{
    pthread_mutex_lock(&counters_lock);
    *ai = asc_iterations;
    *di = desc_iterations;
    *ei = equal_iterations;
    *as = asc_swaps;
    *ds = desc_swaps;
    *es = equal_swaps;
    pthread_mutex_unlock(&counters_lock);
}

typedef int (*pair_predicate)(int l1, int l2);

static int pred_less(int l1, int l2) { return l1 <  l2; }
static int pred_greater(int l1, int l2) { return l1 >  l2; }
static int pred_equal(int l1, int l2) { return l1 == l2; }
static int pred_not_equal(int l1, int l2) { return l1 !=  l2; }

static void traverse_list(Storage *s, pair_predicate match_pred, long *iter_counter, long *pairs_counter)
{
    while (1)
    {
        Node *first = s->first;
        Node *a, *b;

        pthread_spin_lock(&first->sync);
        a = first->next;
        if (!a)
        {
            pthread_spin_unlock(&first->sync);
            inc_long(iter_counter);
            continue;
        }

        pthread_spin_lock(&a->sync);
        pthread_spin_unlock(&first->sync);

        while (1)
        {
            b = a->next;
            if (!b) break;

            pthread_spin_lock(&b->sync);

            int l1 = (int)strlen(a->value);
            int l2 = (int)strlen(b->value);

            if (match_pred(l1, l2)) inc_long(pairs_counter);

            pthread_spin_unlock(&a->sync);
            a = b;
        }

        pthread_spin_unlock(&a->sync);
        inc_long(iter_counter);
    }
}

void *thread_asc(void *arg)
{
    traverse_list((Storage *)arg, pred_less, &asc_iterations, &asc_pairs);
    return NULL;
}

void *thread_desc(void *arg)
{
    traverse_list((Storage *)arg, pred_greater, &desc_iterations, &desc_pairs);
    return NULL;
}

void *thread_equal(void *arg)
{
    traverse_list((Storage *)arg, pred_equal, &equal_iterations, &equal_pairs);
    return NULL;
}

static void do_random_swap(Storage *s, pair_predicate need_swap, long *swap_counter, unsigned int *seed)
{
    if (s->count < 2) return;

    int index = rand_r(seed) % (s->count - 1);

    Node *prev = s->first;
    pthread_spin_lock(&prev->sync);

    Node *a = prev->next;
    if (!a)
    {
        pthread_spin_unlock(&prev->sync);
        return;
    }
    pthread_spin_lock(&a->sync);

    for (int i = 0; i < index; ++i)
    {
        Node *next = a->next;
        if (!next) break;

        pthread_spin_lock(&next->sync);

        pthread_spin_unlock(&prev->sync);
        prev = a;
        a = next;
    }

    Node *b = a->next;
    if (!b)
    {
        pthread_spin_unlock(&a->sync);
        pthread_spin_unlock(&prev->sync);
        return;
    }
    pthread_spin_lock(&b->sync);

    int l1 = (int)strlen(a->value);
    int l2 = (int)strlen(b->value);

    if (need_swap(l1, l2))
    {
        prev->next = b;
        a->next    = b->next;
        b->next    = a;
        inc_long(swap_counter);
    }

    pthread_spin_unlock(&b->sync);
    pthread_spin_unlock(&a->sync);
    pthread_spin_unlock(&prev->sync);
}

void *thread_swap_asc(void *arg)
{
    Storage *s = (Storage *)arg;
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)pthread_self();

    while (1)
    {
        do_random_swap(s, pred_greater, &asc_swaps, &seed);
        sched_yield();
    }
    return NULL;
}

void *thread_swap_desc(void *arg)
{
    Storage *s = (Storage *)arg;
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)pthread_self();

    while (1)
    {
        do_random_swap(s, pred_less, &desc_swaps, &seed);
        sched_yield();
    }
    return NULL;
}

void *thread_swap_equal(void *arg)
{
    Storage *s = (Storage *)arg;
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)pthread_self();

    while (1)
    {
        do_random_swap(s, pred_not_equal, &equal_swaps, &seed);
        sched_yield();
    }
    return NULL;
}

void *thread_monitor(void *arg)
{
    (void)arg;

    while (1)
    {
        long ai, di, ei, as, ds, es;
        snapshot_counters(&ai, &di, &ei, &as, &ds, &es);

        printf("[monitor][spin] iter: asc=%ld desc=%ld equal=%ld | "
               "swaps: asc=%ld desc=%ld equal=%ld\n",
               ai, di, ei, as, ds, es);
        fflush(stdout);
        sleep(2);
    }

    return NULL;
}