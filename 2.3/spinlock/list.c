#define _GNU_SOURCE
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void random_string(char *buf, size_t buf_size)
{
    const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
    size_t alpha_len = sizeof(alphabet) - 1;
    int maxlen = (int)buf_size - 1;

    int len = 1 + rand() % maxlen;
    for (int i = 0; i < len; ++i) buf[i] = alphabet[rand() % alpha_len];
    buf[len] = '\0';
}

Storage *storage_init(int n) 
{
    if (n <= 0) return NULL;

    Storage *s = malloc(sizeof(Storage));
    if (!s) 
    {
        perror("malloc storage");
        exit(1);
    }

    Node *first = malloc(sizeof(Node)):
    if (!first) 
    {
        perror("malloc first");
        exit(1);
    }

    first->value[0] = '\0';
    first->next = NULL;
    if (pthread_spin_init(&first->sync, PTHREAD_PROCESS_PRIVATE) != 0) 
    {
        perror("pthread_spin_init first");
        exit(1);
    }

    s->first = first;
    s->count = n;

    srand((unsigned int)time(NULL));

    Node *prev = NULL;
    for (int i = 0; i < n; i++) 
    {
        Node *node = malloc(sizeof(Node));
        if (!node) 
        {
            perror("malloc node");
            exit(1);
        }

        random_string(node->value, sizeof(node->value));
        node->next = NULL;

        if (pthread_spin_init(&node->sync, PTHREAD_PROCESS_PRIVATE) != 0) 
        {
            perror("pthread_spin_init node");
            exit(1);
        }

        prev->next = node;
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
        pthread_spin_destroy(&cur->sync);
        free(cur);
        cur = next;
    }

    free(s);
}

void storage_print(Storage *s) 
{
    if (!s) return;
    printf("Storage contents (%d nodes):\n", s->count);

    Node *cur = s->first->next;
    int i = 0;
    while (cur) 
    {
        printf("[%03d] \"%s\"\n", i++, cur->value);
        cur = cur->next;
    }
}