#define _GNU_SOURCE
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

int main() 
{
    Storage *s = storage_init(10);
    storage_print(s);
    storage_destroy(s);
    return 0;
}