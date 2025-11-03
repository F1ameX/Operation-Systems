#define _GNU_SOURCE
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

Storage *storage_init(int n) 
{
    if (n <= 0) return NULL;

    Storage *s = malloc(sizeof(Storage));
    if (!s) 
    {
        perror("malloc storage");
        exit(1);
    }

    s->first = NULL;
    s->count = n;

    srand(time(NULL));

    Node *prev = NULL;
    for (int i = 0; i < n; i++) 
    {
        Node *node = malloc(sizeof(Node));
        if (!node) 
        {
            perror("malloc node");
            exit(1);
        }

        int len = rand() % 100 + 1;
        for (int j = 0; j < len; j++) node->value[j] = 'a' + rand() % 26;
        node->value[len] = '\0';

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
        pthread_mutex_init(&node->sync, &attr);
        pthread_mutexattr_destroy(&attr);

        node->next = NULL;

        if (prev == NULL) s->first = node;
        else prev->next = node;

        prev = node;
    }

    return s;
}

void storage_destroy(Storage *s) 
{
    if (!s) return;

    Node *cur = s->first;
    while (cur) 
    {
        Node *next = cur->next;
        pthread_mutex_destroy(&cur->sync);
        free(cur);
        cur = next;
    }

    free(s);
}

void storage_print(Storage *s) 
{
    if (!s) return;
    printf("Storage contents (%d nodes):\n", s->count);

    Node *cur = s->first;
    int i = 0;
    while (cur) 
    {
        printf("[%03d] \"%s\"\n", i++, cur->value);
        cur = cur->next;
    }
}