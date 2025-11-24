#ifndef CACHE_PROXY_THREADPOOL_H
#define CACHE_PROXY_THREADPOOL_H

#include <pthread.h>

typedef void (*threadTaskFn)(void *);

typedef struct threadTask {
    threadTaskFn fn;
    void *arg;
    struct threadTask *next;
} threadTask_t;

typedef struct threadPool {
    int stop;
    int num_threads;
    pthread_t *threads;
    

    threadTask_t *head;
    threadTask_t *tail;

    pthread_mutex_t lock;
    pthread_cond_t  has_work;
} threadPool_t;

threadPool_t *threadPoolCreate(int num_threads);
int           threadPoolSubmit(threadPool_t *pool, threadTaskFn fn, void *arg);
void          threadPoolStop(threadPool_t *pool);

#endif /* CACHE_PROXY_THREADPOOL_H */