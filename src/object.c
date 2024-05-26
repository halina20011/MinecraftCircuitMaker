#include "object.h"

VECTOR_TYPE_FUNCTIONS(struct Block*, BlockPVector);

VECTOR_TYPE_FUNCTIONS(struct Chunk*, ChunkXVector);
// y vector of x 
VECTOR_TYPE_FUNCTIONS(struct ChunkXVector*, ChunkYVector);
// z vector of y
VECTOR_TYPE_FUNCTIONS(struct ChunkYVector*, ChunkZVector);

VECTOR_TYPE_FUNCTIONS(struct Object*, ObjectPVector);
VECTOR_TYPE_FUNCTIONS(size_t, HistogramVector);

// # BLOCK FILE FORMAT
// block id size (uint8_t)
// block id (string)
// vertices size (uint32_t)
// vertices (float * [vertices size])
// texture size (uint32_t)
// texture data (float)

struct BlockSupervisor *blockSupervisorInit(){
    struct BlockSupervisor *bs = malloc(sizeof(struct BlockSupervisor));
    bs->blockTypesNames = blockNames();
    size_t dataSize = 0;
    bool *set;
    // load all blocks that are in the directory
    struct BlockType *blockTypes = loadBlocks(bs, &set, &dataSize);
    // crete a block's type data buffer that will be send to GPU
    // and then later used to draw blocks
    float *modelsData = malloc(sizeof(float) * dataSize);

    // map that maps the block id to the start index of modelsData
    // it will be used to draw the block
    ssize_t *blockDataStartIndex = calloc(BLOCK_TYPES_SIZE, sizeof(ssize_t));

    ssize_t dataSizeEnd = 0;
    for(size_t i = 0; i < BLOCK_TYPES_SIZE; i++){
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
    
    // send the data to GPU and free the buffer
    // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSize, modelsData, GL_STATIC_DRAW);
    // free(modelsData);

    // load blocks textures
    bs->blocksTexture = loadBlocksTexture("Assets/texture.bin");
    printf("blocks texture has id: %i\n", bs->blocksTexture);

    bs->blockDataStartIndex = blockDataStartIndex;

    bs->blockTypes = blockTypes;
    bs->blockTypesHistogram = calloc(BLOCK_TYPES_SIZE, sizeof(size_t));

    // init buildings
    bs->buildings = BuildingPVectorInit();
    bs->buildingsHistogram = HistogramVectorInit();
    
    // load
    bs->chunks = chunksInit();
    bs->chunks->chunks = ChunkXVectorInit();
    bs->blocks = BlockPVectorInit();

    return bs;
}

void boundingBoxPrint(struct BoundingBox *bb){
    printf("(%f %f %f) (%f %f %f)\n", 
        bb->min[0],
        bb->min[1],
        bb->min[2],

        bb->max[0],
        bb->max[1],
        bb->max[2]
    );
}
