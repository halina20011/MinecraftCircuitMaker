#ifndef BLOCK
#define BLOCK

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct Block{
    uint16_t type;
    uint8_t rotation;
};

struct Chunk{
    
};

enum{
    X_PLUST,
    Y_PLUST,
    Z_PLUST,
    X_MINUS,
    Y_MINUS,
    Z_MINUS,
};

struct Building{
    struct Chunk *chunks[6];
    struct Chunk chunksSizes[6];
};

struct ResBuilding{
    size_t width, depth, height;
    struct Block *blocks;
};

void loadBlocks(const char blockString[], size_t size);

void buildingExport(struct Building building);

#endif
