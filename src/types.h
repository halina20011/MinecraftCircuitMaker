#ifndef TYPES
#define TYPES

#include "stdint.h"

typedef char PathType[255];

typedef int32_t BlockPosition[3];
#define blockPosVec3(a) (vec3){a[0], a[1], a[2]}

typedef uint8_t ChunkPosition;

typedef uint16_t BlockId;

#endif
