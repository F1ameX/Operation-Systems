#define _XOPEN_SOURCE 700
#include "uthread.h"
#include <string.h>
#include <stdio.h>

static ucontext_t main_ctx;
static uthread_t *current = NULL;
static uthread_t *readyq = NULL;

static void q_push(uthread_t *t) {t->next = readyq; readyq = t;}

static uthread_t* q_pop_ready(void)
{
    uthread_t **pp = &readyq;
    while (*pp && (*pp)->state != UTHREAD_READY) pp = &(*pp)->next;
    if (!*pp) return NULL;
    uthread_t *t = *pp; *pp = (*pp)->next; t->next = NULL;
    return t;
}

static void uthread_entry(void)
{
    if (current && current->start_routine) current->retval = current->start_routine(current->arg);
    current->state = UTHREAD_FINISHED;
    uthread_yield();
}

int uthread_create(uthread_t *t, void *(*start_routine)(void *), void *arg) 
{
    if(!t || !start_routine) return -1;

    memset(t, 0, sizeof(*t));
    t->stack = malloc(STACK_SIZE);
    if(!t->stack) return -1;

    if (getcontext(&t->context) == -1)
    {
        free(t->stack);
        return -1;
    }

    t->context.uc_stack.ss_sp = t->stack;
    t->context.uc_stack.ss_size = STACK_SIZE;
    t->context.uc_link = &main_ctx;

    t->start_routine = start_routine;
    t->arg = arg;
    t->state = UTHREAD_READY;

    makecontext(&t->context, uthread_entry, 0);

    q_push(t);

    return 0;
}

void uthread_run(uthread_t *t)
{
    current = t;
    t->state = UTHREAD_RUNNING;
    swapcontext(&main_ctx, &t->context);
}

void uthread_yield(void)
{
    uthread_t *prev = current;

    if (prev && prev->state == UTHREAD_RUNNING) 
    {
        prev->state = UTHREAD_READY;
        q_push(prev);
    }

    uthread_t *next = q_pop_ready();
    if (!next) 
    {
        if (prev) swapcontext(&prev->context, &main_ctx);
        return;
    }

    current = next;
    next->state = UTHREAD_RUNNING;

    if (prev) swapcontext(&prev->context, &next->context);
    else swapcontext(&main_ctx, &next->context);
}

void uthread_run_all(void)
{
    while (1) 
    {
        uthread_t *t = q_pop_ready();
        if (!t) break;

        current = t;
        t->state = UTHREAD_RUNNING;
        swapcontext(&main_ctx, &t->context);

        if (t->state == UTHREAD_FINISHED && t->stack) 
        {
            free(t->stack);
            t->stack = NULL;
        } 
        else if (t->state == UTHREAD_READY) q_push(t);
    }
}