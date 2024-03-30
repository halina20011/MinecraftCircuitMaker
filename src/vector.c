#include "vector.h"

struct Vector *vectorInit(){
    struct Vector *v = malloc(sizeof(struct Vector));
    v->maxSize = DEFAULT_VEC_INIT_SIZE;
    v->size = 0;
    v->data = malloc(sizeof(void*) * v->maxSize);

    return v;
}

void vectorPush(struct Vector *v, void *data){
    printf("%zu %zu\n", v->size, v->maxSize);
    if(v->maxSize <= v->size){
        v->maxSize *= 2;
        v->data = realloc(v->data, sizeof(void*) * v->maxSize);
    }

    v->data[v->size++] = data;
}
