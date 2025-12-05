#include "cache/cache.h"
#include "logger/logger.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define CACHE_DEFAULT_BUCKETS 1024

static unsigned long hash_key(const char *s)
{
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*s++) != 0)
        h = ((h << 5) + h) + (unsigned long)c;
    return h;
}


static cache_entry_t *cache_entry_create(const char *key)
{
    cache_entry_t *e = calloc(1, sizeof(*e));
    if (!e) return NULL;

    e->key = strdup(key);
    if (!e->key)
    {
        free(e);
        return NULL;
    }

    e->data     = NULL;
    e->size     = 0;
    e->capacity = 0;
    e->complete = 0;
    e->failed   = 0;
    e->refcnt   = 1;
    e->next     = NULL;

    if (pthread_mutex_init(&e->lock, NULL) != 0)
    {
        free(e->key);
        free(e);
        return NULL;
    }
    if (pthread_cond_init(&e->cond, NULL) != 0)
    {
        pthread_mutex_destroy(&e->lock);
        free(e->key);
        free(e);
        return NULL;
    }

    return e;
}

static void cache_entry_destroy(cache_entry_t *e)
{
    if (!e) return;
    pthread_mutex_destroy(&e->lock);
    pthread_cond_destroy(&e->cond);
    free(e->data);
    free(e->key);
    free(e);
}

/* ============ API ============ */

int cache_table_init(cache_table_t *t)
{
    if (!t) return -1;

    t->nbuckets = CACHE_DEFAULT_BUCKETS;
    t->buckets  = calloc(t->nbuckets, sizeof(cache_entry_t *));
    if (!t->buckets)
        return -1;

    if (pthread_mutex_init(&t->lock, NULL) != 0)
    {
        free(t->buckets);
        t->buckets = NULL;
        return -1;
    }

    log_debug("cache table initialized");
    return 0;
}

void cache_table_destroy(cache_table_t *t)
{
    if (!t || !t->buckets) return;

    for (size_t i = 0; i < t->nbuckets; ++i)
    {
        cache_entry_t *e = t->buckets[i];
        while (e)
        {
            cache_entry_t *next = e->next;
            cache_entry_destroy(e);
            e = next;
        }
    }

    free(t->buckets);
    t->buckets = NULL;
    pthread_mutex_destroy(&t->lock);
}

cache_entry_t *cache_start_or_join(cache_table_t *t, const char *key, int *am_writer)
{
    if (!t || !key || !am_writer) return NULL;

    unsigned long h = hash_key(key);
    size_t idx = h % t->nbuckets;

    pthread_mutex_lock(&t->lock);

    cache_entry_t *e = t->buckets[idx];
    while (e)
    {
        if (strcmp(e->key, key) == 0)
        {
            e->refcnt++;
            *am_writer = 0;
            pthread_mutex_unlock(&t->lock);
            log_debug("cache HIT key='%s', refcnt=%d", key, e->refcnt);
            return e;
        }
        e = e->next;
    }

    e = cache_entry_create(key);
    if (!e)
    {
        pthread_mutex_unlock(&t->lock);
        log_error("cache_start_or_join: failed to create entry for key='%s'", key);
        return NULL;
    }

    e->next = t->buckets[idx];
    t->buckets[idx] = e;

    *am_writer = 1;
    pthread_mutex_unlock(&t->lock);

    log_debug("cache MISS, new entry key='%s'", key);
    return e;
}

int cache_append_data(cache_entry_t *e, const void *data, size_t len)
{
    if (!e || !data || len == 0) return 0;

    pthread_mutex_lock(&e->lock);

    if (e->failed)
    {
        pthread_mutex_unlock(&e->lock);
        return -1;
    }

    size_t need = e->size + len;
    if (need > e->capacity)
    {
        size_t newcap = e->capacity ? e->capacity * 2 : 64 * 1024;
        if (newcap < need)
            newcap = need;

        char *newdata = realloc(e->data, newcap);
        if (!newdata)
        {
            e->failed = 1;
            pthread_cond_broadcast(&e->cond);
            pthread_mutex_unlock(&e->lock);
            log_error("cache_append_data: realloc to %zu bytes failed", newcap);
            return -1;
        }

        e->data     = newdata;
        e->capacity = newcap;
    }

    memcpy(e->data + e->size, data, len);
    e->size += len;

    pthread_cond_broadcast(&e->cond);
    pthread_mutex_unlock(&e->lock);

    return 0;
}

void cache_mark_complete(cache_entry_t *e)
{
    if (!e) return;
    pthread_mutex_lock(&e->lock);
    e->complete = 1;
    pthread_cond_broadcast(&e->cond);
    pthread_mutex_unlock(&e->lock);
}

void cache_mark_failed(cache_entry_t *e)
{
    if (!e) return;
    pthread_mutex_lock(&e->lock);
    e->failed = 1;
    pthread_cond_broadcast(&e->cond);
    pthread_mutex_unlock(&e->lock);
}

void cache_release(cache_table_t *t, cache_entry_t *e)
{
    if (!t || !t->buckets || !e) return;

    pthread_mutex_lock(&t->lock);

    e->refcnt--;
    if (e->refcnt > 0)
    {
        pthread_mutex_unlock(&t->lock);
        log_debug("cache_release key='%s', new refcnt=%d", e->key, e->refcnt);
        return;
    }

    unsigned long h = hash_key(e->key);
    size_t idx = h % t->nbuckets;

    cache_entry_t **pp = &t->buckets[idx];
    while (*pp)
    {
        if (*pp == e)
        {
            *pp = e->next;
            break;
        }
        pp = &(*pp)->next;
    }

    pthread_mutex_unlock(&t->lock);

    log_debug("cache entry fully freed key='%s'", e->key);
    cache_entry_destroy(e);
}