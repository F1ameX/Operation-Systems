#pragma once
#include <stdatomic.h>
#include <stdbool.h>

typedef struct {
    _Atomic int state;
} spinlock_t;

static inline void spinlock_init(spinlock_t *l)
{
    atomic_store_explicit(&l->state, 0, memory_order_relaxed);
}

static inline bool spinlock_trylock(spinlock_t *l)
{
    int expected = 0;
    return atomic_compare_exchange_strong_explicit(&l->state, &expected, 1, memory_order_acquire, memory_order_relaxed);
}

void spinlock_lock(spinlock_t *l);
void spinlock_unlock(spinlock_t *l);