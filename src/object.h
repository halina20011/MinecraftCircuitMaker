#ifndef BLOCK
#define BLOCK

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>

#include <libgen.h> // basename

#define CGLM_DEFINE_PRINTS 1
#define DEBUG 1

#include <cglm/cglm.h>
#include <cglm/types.h>
#include <cglm/io.h>

#include "types.h"
#include "graphics.h"
#include "func.h"

#include "blocks.h"

#define QUADRANTS_SIZE 8

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
    SOUTH,
    NORTH,
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
    BlockTypeId id;
    BlockTypeIdStr *idStr;
    float *data;
    size_t dataSize;

    struct BoundingBox boundingBox;
};

NEW_VECTOR_TYPE(struct Block*, BlockPVector);

struct Block{
    BlockTypeId id;
    BlockPosition position;
    BlockRotation facing;
    BlocksSize index;
};

// BUILDING_FLIP_HORIZONTAL,
// BUILDING_FLIP_VERTICAL,
// BUILDING_FLIP_ZEDICAL
enum BuildingRotation{
    BUILDING_FLIP_WIDTH,
    BUILDING_FLIP_HEIGHT,
    BUILDING_FLIP_DEPTH
};

struct BlockBuilding{
    BlockTypeId id;
    BlockPosition pos;
    BlockRotation facing;
};

struct BuildingType{
    BuildingTypeId id;
    char *name;
    BuildingPath path;
    BuildingPathSize pathSize;
    struct BlockBuilding **blocks;
    BlockTypesSize size;
    struct BoundingBox boundingBox;
};

struct Building{
    BuildingTypeId id;
    size_t index;
    BlockPosition position;
    BlockRotation facing;
};

NEW_VECTOR_TYPE(struct BuildingType*, BuildingTypePVector);

struct Chunk{
    ChunkPosition x, y, z;
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

struct Chunks{
    struct ChunkXVector *chunks;
    struct ChunkZVector *quadrants[QUADRANTS_SIZE];
};

struct BlockSupervisor{
    struct Chunks *chunks;
    struct BlockPVector *blocks;

    struct BlockType *blockTypes;
    BlockTypeId *availableBlockTypes;
    size_t availableBlockTypesSize;
    char **blockTypesNames;
    size_t *blockTypesHistogram;

    struct BuildingTypePVector *buildingTypes;
    struct HistogramVector *buildingTypesHistogram;

    struct BuildingPVector *buildings;
    struct ObjectPVector *objects;

    ssize_t *blockDataStartIndex;
    GLint modelUniformLocatio;
    GLuint vao;
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
NEW_VECTOR_TYPE(struct Building*, BuildingPVector);

struct BlockSupervisor *blockSupervisorInit(struct Shader *shader, GLint modelUniformLocatio);

bool boundingBoxIntersection(struct BoundingBox *bb, vec3 rayOrigin, vec3 rayDirection, mat4 modelMatrix, float *r, uint8_t *intersectionAxis);
void boundingBoxPrint(struct BoundingBox *bb);
void boundingBoxDraw(struct BoundingBox *bb, BlockPosition pos, BlockRotation facing, GLuint arrayBuffer, GLuint elementArrayBuffer, GLuint modelUniformLocation);

float facingToRad(BlockRotation facing);

void exportAsBuilding(struct BlockSupervisor *bs, char path[]);
void exportBlocks(struct BlockSupervisor *bs, FILE *f, bool unwrapp);
void exportBuildings(struct BlockSupervisor *bs, FILE *f);

// BLOCK
struct Block *addBlock(struct BlockSupervisor *bs, BlockTypeId id, BlockPosition pos, BlockRotation facing);
void blockDelete(struct BlockSupervisor *bs, BlockPosition position);
void drawBlocks(struct BlockSupervisor *bs);

// BLOCK TYPE
bool loadBlock(struct BlockSupervisor *bs, const char fileName[], struct BlockType *block);
char **blockIds();
BlockTypeId blockId(struct BlockSupervisor *bs, const char *id);
void blockTypeBoundingBox(struct BoundingBox *bb, float *data, size_t size);
struct BlockType *loadBlocks(struct BlockSupervisor *bs, bool **set, size_t *rDataSize);
GLuint loadBlocksTexture(const char textureFile[]);

// CHUNK
struct Chunk *chunkInit(uint8_t _x, uint8_t _y, uint8_t _z);
struct Chunks *chunksInit();
struct Block **blockPositionToChunkGrid(struct BlockSupervisor *bs, BlockPosition position);
uint8_t positionToQuadrant(struct Chunks *chunks, BlockPosition position, struct ChunkZVector **quadrant);
struct Chunk *positionToChunk(struct BlockSupervisor *bs, BlockPosition position);
void addBlockToChunk(struct Chunk *chunk, struct Block *block);
void drawChunk(struct Chunk *chunk);
void drawChunks(struct BlockSupervisor *bs);

// BUILDING
void buildingAdd(struct BlockSupervisor *bs, BuildingTypeId id, BlockPosition pos, BlockRotation facing);
void buildingDelete(struct BlockSupervisor *bs, struct Building *building);
void buildingDraw(struct BlockSupervisor *bs);
void buildingLoadFromDirectory(struct BlockSupervisor *bs, const char directoryPath[]);
void buildingExport(struct BlockSupervisor *bS, const char fileName[]);
void buildingLoad(struct BlockSupervisor *bs, const char fileName[]);

#endif
