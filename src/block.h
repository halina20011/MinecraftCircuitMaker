#ifndef BLOCK
#define BLOCK

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>

#define CGLM_DEFINE_PRINTS 1
#define DEBUG 1

#include <cglm/cglm.h>
#include <cglm/types.h>
#include <cglm/io.h>

#include "graphics.h"
#include "func.h"

#include "blocks.h"

#define MAX_BLOCK_ID_SIZE UINT8_MAX

#define CHUNK_WIDTH     32
#define CHUNK_DEPTH     32
#define CHUNK_HEIGHT    32


// paths

#ifndef BLOCKS_DIR_PATH
#define BLOCKS_DIR_PATH __BASE_FILE__
#endif

#ifndef BUILDINGS_DIR_PATH
#define BUILDINGS_DIR_PATH __BASE_FILE__
#endif

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
    float min[3];
    float max[3];
};

struct BlockType{
    uint16_t id;
    char *idStr;
    float *data;
    size_t dataSize;
    struct BoundingBox boundingBox;
};

NEW_VECTOR_TYPE(struct Block*, BlockPVector);

struct Block{
    uint16_t blockTypeIndex;
    vec3 position;
    vec3 rotation;

    // int8_t chunkIndex;
    uint8_t x, y, z;
    uint8_t groupIndex;
};

struct Building{
    uint32_t size;
    char *name;
    struct Block **blocks;
};

NEW_VECTOR_TYPE(struct Building*, BuildingPVector);

struct Chunk{
    // uint8_t x, y, z;
    // 3d array to pointer of block
    struct Block ***(*grid);
};

// x vector of chunks
NEW_VECTOR_TYPE(struct Chunk*, ChunkXVector);
// y vector of x 
NEW_VECTOR_TYPE(struct ChunkXVector*, ChunkYVector);
// z vector of y
NEW_VECTOR_TYPE(struct ChunkYVector*, ChunkZVector);

#define QUADRANTS_SIZE 8

struct Chunks{
    struct ChunkZVector *quadrants[QUADRANTS_SIZE];
};

struct BlockSupervisor{
    struct Chunks *chunks;
    struct BlockPVector *blocks;
    struct BlockType *blockTypes;

    struct BuildingPVector *buildings;

    ssize_t *blockDataStartIndex;
    GLuint blocksVBO;
};

GLuint loadAllBlocks(const char textureFile[]);
struct BlockType *loadBlocks(bool **set, size_t *rDataSize);
bool intersection(struct BlockType *block, vec3 rayOrigin, vec3 rayDirection, mat4 modelMatrix, float *r);

void blockSupervisorInit(struct BlockSupervisor *bs);
struct Chunks *chunksInit();

struct Block *addBlock(struct BlockSupervisor *bs, uint16_t id, vec3 pos, vec3 rot);

void drawBlock(struct BlockType blockType);
void drawBlocks(struct BlockSupervisor *bs, GLint modelUniformLocation);

void buildingLoadDirectory(const char directoryPath[]);
void buildingExport(struct BlockSupervisor *bS, const char fileName[]);
void buildingLoad(struct BlockSupervisor *bs, const char fileName[]);

#endif
