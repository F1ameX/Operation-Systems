#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include "spinlock.h"

enum {N_THREADS = 8};
enum {N_ITERS = 1000000};

static volatile long long counter = 0;
static spinlock_t my_lock;
static pthread_spinlock_t p_lock;

static inline double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

static void *worker_race(void *arg)
{
    (void)arg;
    for (int i = 0; i < N_ITERS; ++i) counter++;
    return NULL;
}

static void *worker_my_spin(void *arg)
{
    (void)arg;
    for (int i = 0; i < N_ITERS; ++i) 
    {
        spinlock_lock(&my_lock);
        counter++;
        spinlock_unlock(&my_lock);
    }
    return NULL;
}

static void *worker_pthread_spin(void *arg)
{
    (void)arg;
    for (int i = 0; i < N_ITERS; ++i) 
    {
        pthread_spin_lock(&p_lock);
        counter++;
        pthread_spin_unlock(&p_lock);
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
    printf("%s: counter=%lld (expected=%lld) => %s | %.2f ms\n",
           label,
           (long long)counter,
           expected,
           (counter == expected) ? "OK" : "RACE",
           t1 - t0);
}

int main(void)
{
    spinlock_init(&my_lock);
    pthread_spin_init(&p_lock, PTHREAD_PROCESS_PRIVATE);

    run_test(worker_race, "no-lock");
    run_test(worker_my_spin, "my_spinlock");
    run_test(worker_pthread_spin, "pthread_spinlock");

    pthread_spin_destroy(&p_lock);
    return 0;
}