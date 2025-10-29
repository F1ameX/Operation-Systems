#ifndef UTHREAD_H
#define UTHREAD_H

#include <ucontext.h>
#include <stdlib.h>

#define STACK_SIZE (1 << 20)

typedef enum {
    UTHREAD_READY,
    UTHREAD_RUNNING,
    UTHREAD_FINISHED
} uthread_state_t;

typedef struct uthread {
    ucontext_t context;
    void *stack;
    void *(*start_routine)(void *);
    void *arg;
    void *retval;
    uthread_state_t state;
    struct uthread *next;
} uthread_t;

int  uthread_create(uthread_t *t, void *(*start_routine)(void *), void *arg);