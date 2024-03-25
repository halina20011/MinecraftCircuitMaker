#include <stdio.h>
#include <stdlib.h>

struct Object{
    float *data;
    size_t dataSize;
    float position[3];
    float rotation[3];
};
