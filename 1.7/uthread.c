#define _XOPEN_SOURCE 700
#include "uthread.h"
#include <string.h>
#include <stdio.h>

static ucontext_t main_ctx;
static uthread_t *current = NULL;

static void uthread_entry(void)
{
    if (current && current->start_routine) current->retval = current->start_routine(current->arg);
    current->state = UTHREAD_FINISHED;
    setcontext(&main_ctx);
}

int uthread_create(uthread_t *t, void *(*start_routine)(void *), void *arg) 
{
    if(!t || !start_routine) return -1;

    memset(t, 0, sizeof(*t));
    t->stack = malloc(STACK_SIZE);
    if(!t->stack) return -1;

    if (getcontext(&t->context) == -1)
    {
        perror("getcontext");
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

    return 0;
}

void uthread_run(uthread_t *t)
{
    current = t;
    t->state = UTHREAD_RUNNING;
    swapcontext(&main_ctx, &t->context);
}