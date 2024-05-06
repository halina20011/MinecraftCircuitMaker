#ifndef BLOCK
#define BLOCK

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>

#include "graphics.h"
#include "func.h"

enum BLOCK_STATES{
    // facing
    EAST,
    NORTH,
    SOUTH,
    WEST,
    UP,
    DOWN,

    // extended
    EXTENDED_FALSE,
    EXTENDED_TRUE,

    POWERED,
    UNPOWERED,
};

struct BoundingBox{
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

struct State{
    char *name;
};

struct BlockType{
    uint8_t type;
    float *data;
    size_t dataSize;
    struct BoundingBox boundingBox;
};

NEW_VECTOR_TYPE(struct BlockType, BlockTypeVector)

struct Block{
    char id;
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

GLuint loadAllBlocks(const char textureFile[]);
struct BlockType *loadBlocks(size_t *size);

void buildingExport(struct Building building);

void drawBlock(struct BlockType block);

#endif
