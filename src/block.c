#include "block.h"
#include "blocks.h"
#include <strings.h>

VECTOR_TYPE_FUNCTIONS(struct Block*, BlockPVector, "");

VECTOR_TYPE_FUNCTIONS(struct Chunk*, ChunkXVector, "");
// y vector of x 
VECTOR_TYPE_FUNCTIONS(struct ChunkXVector*, ChunkYVector, "");
// z vector of y
VECTOR_TYPE_FUNCTIONS(struct ChunkYVector*, ChunkZVector, "");

// # BLOCK FILE FORMAT
// block id size (uint8_t)
// block id (string)
// vertices size (uint32_t)
// vertices (float * [vertices size])
// texture size (uint32_t)
// texture data (float)

#define MAX(a, b) ((a < b) ? b : a)
#define MIN(a, b) ((a < b) ? a : b)

void blockSupervisorInit(struct BlockSupervisor *bs){
    size_t dataSize = 0;
    bool *set;
    struct BlockType *blockTypes = loadBlocks(&set, &dataSize);
    float *modelsData = malloc(sizeof(float) * dataSize);

    ssize_t *blockDataStartIndex = calloc(BLOCKS_SIZE, sizeof(ssize_t));

    ssize_t dataSizeEnd = 0;
    for(size_t i = 0; i < BLOCKS_SIZE; i++){
        if(set[i]){
            struct BlockType *blockType = &blockTypes[i];
            size_t startIndex = dataSizeEnd;
            blockDataStartIndex[i] = startIndex;
            printf("%zu %zu %zu\n", startIndex, dataSize, blockType->dataSize);
            memcpy(&modelsData[startIndex], blockType->data, blockType->dataSize);
            dataSizeEnd += blockType->dataSize;
        }
        else{
            blockDataStartIndex[i] = -1;
        }
    }

    // glGenBuffers(1, &bs->blocksVBO);
    // glBindBuffer(GL_ARRAY_BUFFER, bs->blocksVBO);
    // printf("dataSize: %zu\n", dataSize);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSize, modelsData, GL_STATIC_DRAW);
    // free(modelsData);
    // exit(1);

    bs->blockDataStartIndex = blockDataStartIndex;

    bs->blockTypes = blockTypes;

    // init buildings
    bs->buildings = BuildingPVectorInit();
    // load

    bs->chunks = chunksInit();
    bs->blocks = BlockPVectorInit();
}

void findBoundingBox(struct BoundingBox *bb, float *data, size_t size){
    for(size_t i = 0; i < size; i += 5){
        bb->min[0] = MIN(data[i + 0], bb->min[0]);
        bb->min[1] = MIN(data[i + 1], bb->min[1]);
        bb->min[2] = MIN(data[i + 2], bb->min[2]);
        
        bb->max[0] = MAX(data[i + 0], bb->max[0]);
        bb->max[1] = MAX(data[i + 1], bb->max[1]);
        bb->max[2] = MAX(data[i + 2], bb->max[2]);
    }
}

GLuint loadAllBlocks(const char textureFile[]){
    size_t size;
    uint8_t *data = readFile(textureFile, &size);
    printf("texture size %zu\n", size);
    uint16_t w, h;

    memcpy(&w, data, sizeof(uint16_t));
    memcpy(&h, data + sizeof(uint16_t), sizeof(uint16_t));
    
    printf("texture %ux%u\n", w, h);

    return loadTexture(data + sizeof(uint16_t) * 2, w, h);
}

bool loadBlock(const char fileName[], struct BlockType *block){
    FILE *file = fopen(fileName, "rb");
    if(file == NULL){
        fprintf(stderr, "failed to open file %s\n", fileName);
        exit(1);
    }
    
    uint8_t idSize = 0;
    fread(&idSize, sizeof(uint8_t), 1, file);
    // printf("id size %i\n", idSize);

    char idStr[MAX_BLOCK_ID_SIZE + 1];
    fread(&idStr, sizeof(char), idSize, file);
    idStr[idSize] = 0;

    uint16_t id = blockId(idStr);
    if(id == BLOCK_NOT_FOUND){
        return false;
    }

    uint32_t size = 0;
    fread(&size, sizeof(uint32_t), 1, file);

    printf("block size: %i\n", size);
    float *data = malloc(sizeof(float) * size);

    fread(data, sizeof(float), size, file);
    if(feof(file)){
        fprintf(stderr, "file is shorter then the size specified %s\n", fileName);
        exit(1);
    }

    block->id = id;
    block->idStr = strdup(idStr);

    block->data = data;
    block->dataSize = size;

    float *d = block->data;
    for(int i = 0; i < block->dataSize; i += 5){
        printf("%f %f %f | %f %f\n", d[i], d[i + 1], d[i + 2], d[i + 3], d[i + 4]);
    }

    block->boundingBox.min[0] = FLT_MAX;
    block->boundingBox.min[1] = FLT_MAX;
    block->boundingBox.min[2] = FLT_MAX;

    block->boundingBox.max[0] = FLT_MIN;
    block->boundingBox.max[1] = FLT_MIN;
    block->boundingBox.max[2] = FLT_MIN;

    struct BoundingBox *bb = &block->boundingBox;
    findBoundingBox(bb, data, size);

    printf("(%f %f %f) (%f %f %f)\n", 
        block->boundingBox.min[0],
        block->boundingBox.min[1],
        block->boundingBox.min[2],

        block->boundingBox.max[0],
        block->boundingBox.max[1],
        block->boundingBox.max[2]);

    return true;
}

void drawBlock(struct BlockType blockType){
    size_t trigCount = blockType.dataSize / 5;
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * blockType.dataSize, blockType.data, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, trigCount);
}

void drawBlocks(struct BlockSupervisor *bs, GLint modelUniformLocation){
    // glBindBuffer(GL_ARRAY_BUFFER, bs->blocksVBO);

    // for(size_t i = 0; i < BLOCKS_SIZE; i++){
    //     struct BlockType *blockType = &bs->blockTypes[i];
    //     if(blockType != NULL){
    //         printf("block %s %i %zu\n", blockType->idStr, blockType->id, blockType->dataSize);
    //     }
    //     else{
    //         printf("missing\n");
    //     }
    // }
    // fflush(stdout);

    for(size_t i = 0; i < bs->blocks->size; i++){
        struct Block *block = bs->blocks->data[i];
        struct BlockType *blockType = &(bs->blockTypes[block->blockTypeIndex]);
        if(bs->blockDataStartIndex[blockType->id] == -1){
            printf("invalid\n");
            continue;
        }

        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, block->position);
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);
        // float angle = glm_rad(20 * i);
        // float angle = glm_rad(20 * i);
        // glm_rotate(model, angle, (vec3){1, 0.3, 0.5});

        // glm_vec3_rotate(color, angle, (vec3){0.4, -1.4, 2});
        // glUniform3fv(colorUniform, 1, (float*)color);
        // glUniform3f(colorUniform, 0.5, 0.5, 0.5);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        size_t trigCount = blockType->dataSize / 5;
        // printf("%zu\n", blockType)

        // float *d = blockType->data;
        // for(int i = 0; i < blockType->dataSize; i += 5){
        //     printf("%f %f %f | %f %f\n", d[i], d[i + 1], d[i + 2], d[i + 3], d[i + 4]);
        // }
        // GLint index = bs->blockDataStartIndex[blockType->id]/5;
        // printf("index: %i size: %zu\n", index, trigCount);

        // glDrawArrays(GL_TRIANGLES, index, trigCount);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * blockType->dataSize, blockType->data, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, trigCount);
    }
}

struct BlockType *loadBlocks(bool **set, size_t *rDataSize){
    DIR *d = opendir(BLOCKS_DIR_PATH);
    if(!d){
        fprintf(stderr, "failed to open dir: %s\n", BLOCKS_DIR_PATH);
        exit(1);
    }

    struct BlockType *blocks = calloc(BLOCKS_SIZE, sizeof(struct BlockType));

    *set = calloc(BLOCKS_SIZE, sizeof(bool));
    size_t dataSizeEnd = 0;

    struct dirent *dir;
    while((dir = readdir(d)) != NULL){
        if(dir->d_type != DT_DIR){
            char filePath[255];
            strcpy(filePath, BLOCKS_DIR_PATH);
            strcat(filePath, "/");
            strcat(filePath, dir->d_name);

            // printf("%s %i\n", filePath, dir->d_type);
            struct BlockType blockType;
            // printf("%zu\n", dataSizeEnd);
            if(loadBlock(filePath, &blockType)){
                printf("new block %s: %i\n", blockType.idStr, blockType.id);
                uint16_t blockId = blockType.id;
                printf("block id: %i\n", blockId);
                (*set)[blockId] = true;
                blocks[blockId] = blockType;
                dataSizeEnd += blockType.dataSize;
            }
        }
    }
    closedir(d);

    *rDataSize = dataSizeEnd;

    return blocks;
}

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

struct Block *addBlock(struct BlockSupervisor *bs, uint16_t id, vec3 pos, vec3 rot){
    struct Block *block = malloc(sizeof(struct Block));
    block->blockTypeIndex = id;
    ASSIGN3(block->position, pos);
    ASSIGN3(block->rotation, rot);
    block->groupIndex = 0;

    struct ChunkZVector *quadrant = NULL;
    positionToQuadrant(bs->chunks, pos, &quadrant);
    struct Chunk *chunk = positionToChunk(pos, quadrant);
    addBlockToChunk(chunk, block);
    BlockPVectorPush(bs->blocks, block);

    return block;
}

#define T_MIN 0.0f
#define T_MAX 1000f

#define INTERSECTION(axisIndex){ \
    vec3 axis = {modelMatrix[axisIndex][0], modelMatrix[axisIndex][1], modelMatrix[axisIndex][2]};\
    float e = glm_vec3_dot(axis, delta); \
    float f = glm_vec3_dot(rayDirection, axis); \
    if(0.001f < fabs(f)){\
        float t1 = (e + bb.min[axisIndex]) / f;\
        float t2 = (e + bb.max[axisIndex]) / f;\
        if(t2 < t1){\
            float t = t1;\
            t1 = t2;\
            t2 = t;\
        }\
        if(t2 < tMax){\
            tMax = t2;\
        }\
        if(tMin < t1){\
            tMin = t1;\
        }\
        if(tMax < tMin){\
            return false;\
        }\
    }\
    else{\
        if(0.0f < -e + bb.min[axisIndex] || -e + bb.max[axisIndex] < 0.0f){\
            return false;\
        }\
    }\
}

bool intersection(struct BlockType *block, vec3 rayOrigin, vec3 rayDirection, mat4 modelMatrix, float *r){
    struct BoundingBox bb = block->boundingBox;

    vec3 obbPos = {modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]};
    vec3 delta = {};
    glm_vec3_sub(obbPos, rayOrigin, delta);

    float tMin = 0.0f;
    float tMax = 1000.0f;

    INTERSECTION(0);
    INTERSECTION(1);
    INTERSECTION(2);

    *r = tMin;
    return true;
}
