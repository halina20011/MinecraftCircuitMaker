#include "object.h"

struct Chunk *chunkInit(){
    struct Chunk *chunk = malloc(sizeof(struct Chunk));
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

void positionToQuadrant(struct Chunks *chunks, vec3 position, struct ChunkZVector **quadrant){
    float x = position[1], y = position[2];
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

#define RESIZE_CHUNK(index, chunk, resizeFunc, newItemFunc)do{\
    if(chunk->size <= index){\
        size_t from = chunk->size;\
        /*printf("resizing from %zu to %zu\n", from, index);*/\
        resizeFunc(chunk, index + 1);\
        for(size_t i = from; i < index + 1; i++){\
            chunk->data[i] = NULL;\
        }\
        chunk->data[index] = newItemFunc();\
    }\
}while(0)

struct Chunk *positionToChunk(vec3 position, struct ChunkZVector *quadrant){
    size_t z = labs(lroundf(position[2]));
    size_t y = labs(lroundf(position[1]));
    size_t x = labs(lroundf(position[0]));
    
    RESIZE_CHUNK(z, quadrant, ChunkZVectorResize, ChunkYVectorInit);
    
    struct ChunkYVector *chunkY = quadrant->data[z];
    RESIZE_CHUNK(y, chunkY, ChunkYVectorResize, ChunkXVectorInit);

    struct ChunkXVector *chunkX = chunkY->data[y];
    RESIZE_CHUNK(x, chunkX, ChunkXVectorResize, chunkInit);

    return chunkX->data[x];
}

#define FORMAT_INDEX(val, rVal, size){\
    long index = lroundf(val);\
    if(val < 0){\
        index = -(index + 1);\
    }\
    rVal = (index % size);\
}

void addBlockToChunk(struct Chunk *chunk, struct Block *block){
    uint8_t x, y, z; 
    FORMAT_INDEX(block->position[0], x, CHUNK_WIDTH);
    FORMAT_INDEX(block->position[1], y, CHUNK_DEPTH);
    FORMAT_INDEX(block->position[2], z, CHUNK_HEIGHT);

    chunk->grid[z][y][x] = block;
}
