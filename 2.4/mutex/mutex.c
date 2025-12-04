#include <errno.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <unistd.h>
#include "mutex.h"

static inline int futex_wait(_Atomic int *addr, int expected) 
{
    return syscall(SYS_futex, addr, FUTEX_WAIT, expected, NULL, NULL, 0);
}

static inline int futex_wake(_Atomic int *addr, int n) 
{
    return syscall(SYS_futex, addr, FUTEX_WAKE, n, NULL, NULL, 0);
}

void mutex_lock(mutex_t *m) {
    if (mutex_trylock(m)) return;

    for (;;) 
    {
        int state = atomic_load_explicit(&m->state, memory_order_relaxed);
        if (state == 1 && atomic_compare_exchange_strong_explicit(&m->state, &state, 2, memory_order_relaxed, memory_order_relaxed)) 
        futex_wait(&m->state, 2);
        if (mutex_trylock(m)) return;
    }
}

void mutex_unlock(mutex_t *m) 
{
    int prev = atomic_exchange_explicit(&m->state, 0, memory_order_release);
    if (prev == 2) futex_wake(&m->state, 1);
}