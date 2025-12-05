#ifndef CACHE_CACHE_H
#define CACHE_CACHE_H

#include <pthread.h>
#include <stddef.h>

typedef struct cache_entry
{
    char  *key; 
    char  *data;
    size_t size;
    size_t capacity;
    int complete;
    int failed;
    int refcnt;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    struct cache_entry *next;
} cache_entry_t;

typedef struct cache_table
{
    cache_entry_t   **buckets;
    size_t            nbuckets;
    pthread_mutex_t   lock;
} cache_table_t;

int  cache_table_init(cache_table_t *t);
void cache_table_destroy(cache_table_t *t);

cache_entry_t *cache_start_or_join(cache_table_t *t,
                                   const char *key,
                                   int *am_writer);

int  cache_append_data(cache_entry_t *e, const void *data, size_t len);

void cache_mark_complete(cache_entry_t *e);
void cache_mark_failed(cache_entry_t *e);

void cache_release(cache_table_t *t, cache_entry_t *e);

#endif /* CACHE_CACHE_H */