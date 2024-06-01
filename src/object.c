#include "object.h"
#include "graphics.h"

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

struct BlockSupervisor *blockSupervisorInit(struct Shader *shader, GLint modelUniformLocatio){
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
            memcpy(&modelsData[startIndex], blockType->data, sizeof(float) * blockType->dataSize);
            dataSizeEnd += blockType->dataSize;
            bs->availableBlockTypes[index++] = i;
        }
        else{
            blockDataStartIndex[i] = -1;
        }
    }
    bs->availableBlockTypesSize = index;

    glGenVertexArrays(1, &bs->vao);
    glBindVertexArray(bs->vao);
    glGenBuffers(1, &bs->blocksVBO);
    glBindBuffer(GL_ARRAY_BUFFER, bs->blocksVBO);

    bs->modelUniformLocatio = modelUniformLocatio;
    // printf("dataSize: %zu\n", dataSize);

    GLint posAttrib = glGetAttribLocation(shader->program, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
    GLint textAttrib = glGetAttribLocation(shader->program, "texture");
    glVertexAttribPointer(textAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(posAttrib);
    glEnableVertexAttribArray(textAttrib);

    glDisable(GL_CULL_FACE);
    // send the data to GPU and free the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSize, modelsData, GL_STATIC_DRAW);
    free(modelsData);

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

    buildingLoadFromDirectory(bs, BUILDINGS_DIR_PATH);
    
    // load
    bs->chunks = chunksInit();
    bs->chunks->chunks = ChunkXVectorInit();
    bs->blocks = BlockPVectorInit();

    return bs;
}

void printVec3(vec3 v){
    printf("%f %f %f\n", v[0], v[1], v[2]);
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

float facingToRad(BlockRotation facing){
    switch(facing){
        case EAST:
            return 0;
        case SOUTH:
            return -90;
        case WEST:
            return 180;
        case NORTH:
            return 90;
    }

    return 0;
}

void boundingBoxDraw(struct BoundingBox *bb, BlockPosition pos, BlockRotation facing, GLuint arrayBuffer, GLuint elementArrayBuffer, GLuint modelUniformLocation){
    vec3 a1 = {bb->min[0] + pos[0], bb->min[1] + pos[1], bb->min[2] + pos[2]};
    vec3 b1 = {bb->max[0] + pos[0], bb->min[1] + pos[1], bb->min[2] + pos[2]};
    vec3 c1 = {bb->max[0] + pos[0], bb->min[1] + pos[1], bb->max[2] + pos[2]};
    vec3 d1 = {bb->min[0] + pos[0], bb->min[1] + pos[1], bb->max[2] + pos[2]};

    vec3 a2 = {bb->min[0] + pos[0], bb->max[1] + pos[1], bb->min[2] + pos[2]};
    vec3 b2 = {bb->max[0] + pos[0], bb->max[1] + pos[1], bb->min[2] + pos[2]};
    vec3 c2 = {bb->max[0] + pos[0], bb->max[1] + pos[1], bb->max[2] + pos[2]};
    vec3 d2 = {bb->min[0] + pos[0], bb->max[1] + pos[1], bb->max[2] + pos[2]};

    vec3 offset = {pos[0], pos[1], pos[2]};
    float rot = glm_rad(facingToRad(facing));

    // drawLineWeight(a, b, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(a1, b1, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(b1, c1, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(c1, d1, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(d1, a1, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    
    drawLineWeight(a2, b2, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(b2, c2, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(c2, d2, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(d2, a2, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    
    drawLineWeight(a1, a2, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(b1, b2, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(c1, c2, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(d1, d2, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);

    drawLineWeight(b1, c2, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
    drawLineWeight(c1, b2, offset, rot, arrayBuffer, elementArrayBuffer, modelUniformLocation);
}

#define INTERSECTION(axisIndex){ \
    vec3 axis = {modelMatrix[axisIndex][0], modelMatrix[axisIndex][1], modelMatrix[axisIndex][2]};\
    float e = glm_vec3_dot(axis, delta); \
    float f = glm_vec3_dot(rayDirection, axis); \
    if(0.001f < fabs(f)){\
        float t1 = (e + bb->min[axisIndex]) / f;\
        float t2 = (e + bb->max[axisIndex]) / f;\
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
            *intersectionAxis = axisIndex;\
        }\
        if(tMax < tMin){\
            return false;\
        }\
    }\
    else{\
        if(0.0f < -e + bb->min[axisIndex] || -e + bb->max[axisIndex] < 0.0f){\
            return false;\
        }\
    }\
}

bool boundingBoxIntersection(struct BoundingBox *bb, vec3 rayOrigin, vec3 rayDirection, mat4 modelMatrix, float *r, uint8_t *intersectionAxis){
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
