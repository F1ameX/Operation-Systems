#pragma once
#include <stdatomic.h>
#include <stdbool.h>

typedef struct {
    _Atomic int state;
} mutex_t;

static inline void mutex_init(mutex_t *m) 
{
    atomic_store_explicit(&m->state, 0, memory_order_relaxed);
}

static inline bool mutex_trylock(mutex_t *m) 
{
    int expected = 0;
    return atomic_compare_exchange_strong_explicit(&m->state, &expected, 1, memory_order_acquire, memory_order_relaxed);
}

void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);