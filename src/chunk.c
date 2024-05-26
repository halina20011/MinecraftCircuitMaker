#include "object.h"

struct Chunk *chunkInit(uint8_t _x, uint8_t _y, uint8_t _z){
    struct Chunk *chunk = malloc(sizeof(struct Chunk));
    
    printf("new chunk %i %i %i\n", _x, _y, _z);
    chunk->x = _x;
    chunk->y = _y;
    chunk->z = _z;

    struct Block ****grid = malloc(sizeof(struct Block***) * CHUNK_HEIGHT);
    for(uint8_t z = 0; z < CHUNK_HEIGHT; z++){
        grid[z] = malloc(sizeof(struct Block**) * CHUNK_DEPTH);
        for(uint8_t y = 0; y < CHUNK_DEPTH; y++){
            grid[z][y] = malloc(sizeof(struct Block*) * CHUNK_WIDTH);
            for(uint8_t x = 0; x < CHUNK_WIDTH; x++){
                grid[z][y][x] = NULL;
            }
        }
    }

    chunk->grid = grid;
    return chunk;
}

struct Chunks *chunksInit(){
    struct Chunks *chunks = malloc(sizeof(struct Chunks));
    for(size_t i = 0; i < QUADRANTS_SIZE; i++){
        chunks->quadrants[i] = ChunkZVectorInit();
    }

    return chunks;
}

void positionToQuadrant(struct Chunks *chunks, BlockPosition position, struct ChunkZVector **quadrant){
    float x = position[0], y = position[1];
    bool negative = (position[2] < 0);
    size_t index = 0;
    if(x < 0 && y < 0){
        index = 2;
    }
    else if(y < 0){
        index = 3;
    }
    else if(x < 0){
        index = 1;
    }
    else{
        index = 0;
    }

    *quadrant = chunks->quadrants[negative * 4 + index];
}

#define RESIZE_CHUNK(index, chunk, vectorPush, newItemFunc, ...)do{\
    if(chunk->size <= index){\
        while(chunk->size < index){\
            vectorPush(chunk, NULL);\
        }\
        vectorPush(chunk, newItemFunc(__VA_ARGS__));\
    }\
}while(0)

struct Chunk *positionToChunk(struct BlockSupervisor *bs, BlockPosition position){
    struct ChunkZVector *quadrant = NULL;
    positionToQuadrant(bs->chunks, position, &quadrant);


    int8_t z = position[0] / CHUNK_HEIGHT;
    int8_t y = position[1] / CHUNK_DEPTH;
    int8_t x = position[2] / CHUNK_WIDTH;
    
    printf("position to chunk: (%i %i %i) => %i %i %i\n", position[0], position[1], position[2], x, y, z);
    
    RESIZE_CHUNK(z, quadrant, ChunkZVectorPush, ChunkYVectorInit);
    
    struct ChunkYVector *chunkY = quadrant->data[z];
    RESIZE_CHUNK(y, chunkY, ChunkYVectorPush, ChunkXVectorInit);

    struct ChunkXVector *chunkX = chunkY->data[y];
    bool newChunk = chunkX->size <= x;
    printf("%zu %o %i\n", chunkX->size, x, newChunk);
    RESIZE_CHUNK(x, chunkX, ChunkXVectorPush, chunkInit, x, y, z);
    if(newChunk){
        ChunkXVectorPush(bs->chunks->chunks, chunkX->data[x]);
    }

    return chunkX->data[x];
}

#define FORMAT_INDEX(val, rVal, size){\
    long index = lroundf(val);\
    if(val < 0){\
        index = -(index + 1);\
    }\
    rVal = (index % size);\
}

bool placeIsEmpty(struct BlockSupervisor *bs, BlockPosition position){
    struct Chunk *chunk = positionToChunk(bs, position);

    uint8_t x, y, z; 
    FORMAT_INDEX(position[0], x, CHUNK_WIDTH);
    FORMAT_INDEX(position[2], y, CHUNK_DEPTH);
    FORMAT_INDEX(position[3], z, CHUNK_HEIGHT);

    return (chunk->grid[z][y][x] == NULL);
}

void addBlockToChunk(struct Chunk *chunk, struct Block *block){
    uint8_t x, y, z; 
    FORMAT_INDEX(block->position[0], x, CHUNK_WIDTH);
    FORMAT_INDEX(block->position[1], y, CHUNK_DEPTH);
    FORMAT_INDEX(block->position[2], z, CHUNK_HEIGHT);

    chunk->grid[z][y][x] = block;
}

void drawChunk(struct Chunk *chunk){
    // printf("%f %f %f\n", (float)chunk->x, (float)chunk->y, (float)chunk->z);
    for(uint8_t z = 0; z < CHUNK_HEIGHT; z++){
        drawLine(chunk->x, chunk->y, -z, chunk->x + CHUNK_DEPTH, chunk->y, -z);
    }
}

void drawChunks(struct BlockSupervisor *bs){
    struct ChunkXVector *vector = bs->chunks->chunks;
    // printf("%zu\n", vector->size);
    for(uint8_t i = 0; i < vector->size; i++){
        drawChunk(vector->data[i]);
    }
}
