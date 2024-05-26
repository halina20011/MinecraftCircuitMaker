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

#define QUADRANTS_SIZE 8

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

typedef int32_t BlockPosition[3];

// struct BlockPosition{
//     int16_t [];
// };

#define blockPosVec3(a) (vec3){a[0], a[1], a[2]}

struct Block{
    uint16_t id;
    BlockPosition position;
    uint8_t facing;
};

// BUILDING_FLIP_HORIZONTAL,
// BUILDING_FLIP_VERTICAL,
// BUILDING_FLIP_ZEDICAL
enum BuildingRotation{
    BUILDING_FLIP_WIDTH,
    BUILDING_FLIP_HEIGHT,
    BUILDING_FLIP_DEPTH
};

struct Building{
    uint32_t size;
    char *name;
    struct Block **blocks;
};

NEW_VECTOR_TYPE(struct Building*, BuildingPVector);

struct Chunk{
    uint8_t x, y, z;
    // 3d array to pointer of block
    struct Block ***(*grid);
};

// x vector of chunks
NEW_VECTOR_TYPE(struct Chunk*, ChunkXVector);
// y vector of x 
NEW_VECTOR_TYPE(struct ChunkXVector*, ChunkYVector);
// z vector of y
NEW_VECTOR_TYPE(struct ChunkYVector*, ChunkZVector);

struct Chunks{
    struct ChunkXVector *chunks;
    struct ChunkZVector *quadrants[QUADRANTS_SIZE];
};

struct BlockSupervisor{
    struct Chunks *chunks;
    struct BlockPVector *blocks;

    struct BlockType *blockTypes;
    char **blockTypesNames;
    size_t *blockTypesHistogram;

    struct ObjectPVector *objects;

    struct BuildingPVector *buildings;
    struct HistogramVector *buildingsHistogram;

    ssize_t *blockDataStartIndex;
    GLuint blocksVBO;
    GLuint blocksTexture;
};

NEW_VECTOR_TYPE(size_t, HistogramVector);

enum ObjectType{
    OBJECT_TYPE_BLOCK,
    OBJECT_TYPE_BUILDING
};

struct Object{
    char *name;
    struct BoundingBox *bb;
    void *object;
    enum ObjectType objectType;
};

NEW_VECTOR_TYPE(struct Object*, ObjectPVector);

struct BlockSupervisor *blockSupervisorInit();

// BLOCK
struct Block *addBlock(struct BlockSupervisor *bs, uint16_t id, BlockPosition pos, uint8_t facing);
void drawBlock(struct BlockType blockType);
void drawBlocks(struct BlockSupervisor *bs, GLint modelUniformLocation, GLint textureUniform);
bool blockIntersection(struct BlockType *block, vec3 rayOrigin, vec3 rayDirection, mat4 modelMatrix, float *r, uint8_t *intersectionAxis);

// BLOCK TYPE
bool loadBlock(struct BlockSupervisor *bs, const char fileName[], struct BlockType *block);
char **blockNames();
uint16_t blockId(struct BlockSupervisor *bs, const char *id);
void blockTypeBoundingBox(struct BoundingBox *bb, float *data, size_t size);
struct BlockType *loadBlocks(struct BlockSupervisor *bs, bool **set, size_t *rDataSize);
GLuint loadBlocksTexture(const char textureFile[]);

// CHUNK
struct Chunk *chunkInit(uint8_t _x, uint8_t _y, uint8_t _z);
struct Chunks *chunksInit();
bool placeIsEmpty(struct BlockSupervisor *bs, BlockPosition position);
void positionToQuadrant(struct Chunks *chunks, BlockPosition position, struct ChunkZVector **quadrant);
struct Chunk *positionToChunk(struct BlockSupervisor *bs, BlockPosition position);
void addBlockToChunk(struct Chunk *chunk, struct Block *block);
void drawChunk(struct Chunk *chunk);
void drawChunks(struct BlockSupervisor *bs);

// BUILDING
void buildingLoadDirectory(const char directoryPath[]);
void buildingExport(struct BlockSupervisor *bS, const char fileName[]);
void buildingLoad(struct BlockSupervisor *bs, const char fileName[]);

#endif
