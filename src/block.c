#include "object.h"

#define T_MIN 0.0f
#define T_MAX 1000f

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

void drawBlock(struct BlockType blockType){
    size_t trigCount = blockType.dataSize / 5;
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * blockType.dataSize, blockType.data, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, trigCount);
}

void drawBlocks(struct BlockSupervisor *bs, GLint modelUniformLocation, GLint textureUniform){
    glActiveTexture(GL_TEXTURE0 + bs->blocksTexture);
    glBindTexture(GL_TEXTURE_2D, bs->blocksTexture);
    glUniform1i(textureUniform, bs->blocksTexture);

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

bool blockIntersection(struct BlockType *block, vec3 rayOrigin, vec3 rayDirection, mat4 modelMatrix, float *r){
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
