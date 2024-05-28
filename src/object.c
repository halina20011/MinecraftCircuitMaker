#include "object.h"

VECTOR_TYPE_FUNCTIONS(struct Block*, BlockPVector);

VECTOR_TYPE_FUNCTIONS(struct Chunk*, ChunkXVector);
// y vector of x 
VECTOR_TYPE_FUNCTIONS(struct ChunkXVector*, ChunkYVector);
// z vector of y
VECTOR_TYPE_FUNCTIONS(struct ChunkYVector*, ChunkZVector);

VECTOR_TYPE_FUNCTIONS(struct Object*, ObjectPVector);
VECTOR_TYPE_FUNCTIONS(struct Building*, BuildingPVector);
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
    bs->availableBlockTypes = malloc(sizeof(BlockTypeId) * BLOCK_TYPES_SIZE);

    // crete a block's type data buffer that will be send to GPU
    // and then later used to draw blocks
    float *modelsData = malloc(sizeof(float) * dataSize);

    // map that maps the block id to the start index of modelsData
    // it will be used to draw the block
    ssize_t *blockDataStartIndex = calloc(BLOCK_TYPES_SIZE, sizeof(ssize_t));

    ssize_t dataSizeEnd = 0;
    uint16_t index = 0;
    for(size_t i = 0; i < BLOCK_TYPES_SIZE; i++){
        if(set[i]){
            struct BlockType *blockType = &blockTypes[i];
            size_t startIndex = dataSizeEnd;
            blockDataStartIndex[i] = startIndex;
            printf("%zu %zu %zu\n", startIndex, dataSize, blockType->dataSize);
            memcpy(&modelsData[startIndex], blockType->data, blockType->dataSize);
            dataSizeEnd += blockType->dataSize;
            bs->availableBlockTypes[index++] = i;
        }
        else{
            blockDataStartIndex[i] = -1;
        }
    }
    bs->availableBlockTypesSize = index;

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
    bs->buildingTypes = BuildingTypePVectorInit();
    bs->buildingTypesHistogram = HistogramVectorInit();

    bs->buildings = BuildingPVectorInit();
    bs->objects = ObjectPVectorInit();

    buildingLoadFromDirectory(bs, BLOCKS_DIR_PATH);
    
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

// export options
//  - building has only blocks, all buildings are unwrapped
//  - scene can have blocks and buildings

void exportAsBuilding(struct BlockSupervisor *bs, char pat[]){
    const char path[] = "/tmp/build";
    // char *buildingName = basename(path);
    // BuildingPathSize buildingNameSize = strlen(buildingName);
    // if(!buildingName){
    //     fprintf(stderr, "failed to get building name from path '%s'\n", path);
    //     return;
    // }

    FILE *f = fopen(path, "wb");
    if(!f){
        fprintf(stderr, "failed to open file '%s'\n", path);
        return;
    }

    // fwrite(&buildingNameSize, sizeof(BuildingPathSize), 1, f);
    // fwrite(buildingName, sizeof(BuildingPath), 1, f);
    exportBlocks(bs, f, true);

    printf("exported succesfully\n");
    fclose(f);
}

void exportBlocks(struct BlockSupervisor *bs, FILE *f, bool unwrapp){
    // loop throw all block types and figure out which one are being used
    BlockTypeId blockTypesSize = 0;
    for(size_t i = 0; i < BLOCK_TYPES_SIZE; i++){
        if(bs->blockTypesHistogram[i]){
            blockTypesSize++;
        }
    }

    BlockTypeId *blockTypesRemap = malloc(sizeof(BlockTypesSize) * BLOCK_TYPES_SIZE);
    printf("number of blockTypes %i\n", blockTypesSize);
    
    // export the number of blockTypes
    fwrite(&blockTypesSize, sizeof(BlockTypesSize), 1, f);

    BlockTypeId blockTypesRemapSize = 0;
    // export each used block type
    for(size_t i = 0; i < BLOCK_TYPES_SIZE; i++){
        if(bs->blockTypesHistogram[i]){
            struct BlockType *blockType = &bs->blockTypes[i];
            printf("%s block used\n", blockType->idStr);
            uint8_t idStrSize = strlen(blockType->idStr);
            // export
            fwrite(&idStrSize, sizeof(BlockTypeIdStrSize), 1, f);
            fwrite(blockType->idStr, sizeof(BlockTypeIdStr), idStrSize, f);
            blockTypesRemap[i] = blockTypesRemapSize++;
        }
    }

    // export each block
    BlocksSize numberOfBlocks = bs->blocks->size;
    printf("number of blocks %i\n", numberOfBlocks);
    fwrite(&numberOfBlocks, sizeof(BlocksSize), 1, f);
    for(size_t i = 0; i < numberOfBlocks; i++){
        struct Block *block = bs->blocks->data[i];
        // struct Block *block = b->object;

        // export block
        BlockTypeId id = blockTypesRemap[block->id];
        fwrite(&id, sizeof(BlockTypeId), 1, f);
        fwrite(&block->position, sizeof(BlockPosition), 1, f);
        fwrite(&block->facing, sizeof(BlockRotation), 1, f);
        printf("block id %i => %i\n", block->id, id);
    }
    printf("end blocks\n");
}

void exportBuildings(struct BlockSupervisor *bs, FILE *f){

}
