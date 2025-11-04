#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include "mutex.h"

enum { N_THREADS = 16 };
enum { N_ITERS   = 1000000 };

static volatile long long counter = 0;
static mutex_t mtx;

static inline double now_ms(void) 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

static void *worker_race(void *_)
{
    for (int i = 0; i < N_ITERS; ++i) counter++;
    return NULL;
}

static void *worker_mutex(void *_) 
{
    for (int i = 0; i < N_ITERS; ++i) 
    {
        mutex_lock(&mtx);
        counter++;
        mutex_unlock(&mtx);
    }
    return NULL;
}

static void run_test(void *(*fn)(void *), const char *label) 
{
    pthread_t th[N_THREADS];
    counter = 0;

    double t0 = now_ms();
    for (int i = 0; i < N_THREADS; ++i) pthread_create(&th[i], NULL, fn, NULL);
    for (int i = 0; i < N_THREADS; ++i) pthread_join(th[i], NULL);
    double t1 = now_ms();

    long long expected = (long long)N_THREADS * N_ITERS;
    printf("%s: counter=%lld (expected=%lld) => %s | %.2f ms\n", label, (long long)counter, expected, (counter == expected) ? "OK" : "RACE", t1 - t0);
}

int main(void) 
{
    mutex_init(&mtx);

    run_test(worker_race,  "no-lock");
    run_test(worker_mutex, "mutex");

    return 0;
}