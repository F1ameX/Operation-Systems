#include <stdio.h>
#include <pthread.h>
#include "spinlock.h"

enum { N_THREADS = 8 };
enum { N_ITERS   = 200000 };

static volatile long long counter = 0;
static spinlock_t lock_;

static void *worker_race(void *_)
{
    for (int i = 0; i < N_ITERS; ++i) counter++;
    return NULL;
}

static void *worker_spin(void *_)
{
    for (int i = 0; i < N_ITERS; ++i) {
        spinlock_lock(&lock_);
        counter++;
        spinlock_unlock(&lock_);
    }
    return NULL;
}

static void run_test(void *(*fn)(void *), const char *label)
{
    pthread_t th[N_THREADS];
    counter = 0;

    for (int i = 0; i < N_THREADS; ++i) pthread_create(&th[i], NULL, fn, NULL);
    for (int i = 0; i < N_THREADS; ++i) pthread_join(th[i], NULL);

    long long expected = (long long)N_THREADS * N_ITERS;
    printf("%s: counter=%lld (expected=%lld) => %s\n", label, (long long)counter, expected, (counter == expected) ? "OK" : "RACE");
}

int main(void)
{
    spinlock_init(&lock_);

    run_test(worker_race, "no-lock");
    run_test(worker_spin, "spinlock");

    return 0;
}