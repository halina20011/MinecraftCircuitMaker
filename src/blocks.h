#ifndef BLOCKS
#define BLOCKS

#include <stdio.h>
#include <string.h>
#include <stdint.h>

enum BLOCKS{
    REDSTONE_WIRE,
    REDSTONE_TORCH,
    PISTON,
    STICKY_PISTON,
    TARGET,

    BLOCK_TYPES_SIZE
};

#define BLOCK_NOT_FOUND UINT16_MAX

#endif
