#include <sched.h>
#include "spinlock.h"

static inline void cpu_relax(void)
{
#if defined(__x86_64__) 
    __asm__ __volatile__("pause");
#elif defined(__aarch64__) || defined(__arm__)
    __asm__ __volatile__("yield");
#else
    __asm__ __volatile__("" ::: "memory");
#endif
}

void spinlock_lock(spinlock_t *l)
{
    if (spinlock_trylock(l)) return;

    int spins = 16;
    const int max_spins = 1 << 12;
    int yields = 0;

    for (;;) 
    {
        for (int i = 0; i < spins; ++i) 
        {
            if (spinlock_trylock(l)) return;
            cpu_relax();
        }
        if (spins < max_spins) spins <<= 1;
        if ((++yields & 0x7) == 0) sched_yield();
    }
}

void spinlock_unlock(spinlock_t *l)
{
    atomic_store_explicit(&l->state, 0, memory_order_release);
}