#define _XOPEN_SOURCE 700
#include "uthread.h"
#include <string.h>
#include <stdio.h>


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
    t->context.uc_link = NULL;

    t->start_routine = start_routine;
    t->arg = arg;
    t->state = UTHREAD_READY;

    return 0;
}