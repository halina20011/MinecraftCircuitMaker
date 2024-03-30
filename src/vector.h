#ifndef __VECTOR__
#define __VECTOR__

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_VEC_INIT_SIZE 20

struct Vector{
    void **data;
    size_t size, maxSize;
};

struct Vector *vectorInit();
void vectorPush(struct Vector *v, void *data);

#endif
