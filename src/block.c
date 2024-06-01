#include "object.h"

#define T_MIN 0.0f
#define T_MAX 1000f

struct Block *addBlock(struct BlockSupervisor *bs, BlockTypeId id, BlockPosition pos, BlockRotation facing){
    blockDelete(bs, pos);

    struct Block *block = malloc(sizeof(struct Block));
    block->id = id;

    ASSIGN3(block->position, pos);
    block->facing = facing;
    // ASSIGN3(block->rotation, rot);

    struct Block **blockPosition = blockPositionToChunkGrid(bs, pos);
    *blockPosition = block;
    // addBlockToChunk(chunk, block);

    block->index = bs->blocks->size;
    BlockPVectorPush(bs->blocks, block);

    bs->blockTypesHistogram[id]++;
    // printf("%i %zu\n", id, bs->blockTypesHistogram[id]);

    return block;
}

void blockDelete(struct BlockSupervisor *bs, BlockPosition position){
    struct Block **blockPointer = blockPositionToChunkGrid(bs, position);
    if(*blockPointer == NULL){
        return;
    }

    struct Block *block = *blockPointer;
    *blockPointer = NULL;

    // swap the block with the last one if needed
    if(bs->blocks->size != 1){
        struct Block *lastBlock = bs->blocks->data[bs->blocks->size - 1];
        lastBlock->index = block->index;
        bs->blocks->data[lastBlock->index] = lastBlock;
    }

    bs->blocks->size--;
    // printf("freeing block %p\n", block);
    free(block);
}

void drawBlocks(struct BlockSupervisor *bs){
    // glActiveTexture(GL_TEXTURE0 + bs->blocksTexture);
    // glBindTexture(GL_TEXTURE_2D, bs->blocksTexture);
    // glUniform1i(textureUniform, bs->blocksTexture);

    glBindVertexArray(bs->vao);
    glBindBuffer(GL_ARRAY_BUFFER, bs->blocksVBO);

    for(size_t i = 0; i < bs->blocks->size; i++){
        struct Block *block = bs->blocks->data[i];
        struct BlockType *blockType = &(bs->blockTypes[block->id]);
        if(bs->blockDataStartIndex[blockType->id] == -1){
            fprintf(stderr, "block %i is invalid, skipping\n", blockType->id);
            continue;
        }

        mat4 model;
        glm_mat4_identity(model);

        glm_translate(model, blockPosVec3(block->position));
        vec3 rotation = {};
        switch(block->facing){
            case EAST:
                break;
            case SOUTH:
                rotation[1] = -90;
                break;
            case WEST:
                rotation[1] = 180;
                break;
            case NORTH:
                rotation[1] = 90;
                break;

            case UP:
                rotation[2] = 90;
                break;
            case DOWN:
                rotation[2] = -90;
                break;
        }
        glm_rotate_y(model, glm_rad(rotation[1]), model);
        glm_rotate_z(model, glm_rad(rotation[2]), model);
        glUniformMatrix4fv(bs->modelUniformLocatio, 1, GL_FALSE, (float*)model);
        
        size_t trigCount = blockType->dataSize / 5;
        GLint index = bs->blockDataStartIndex[blockType->id] / 5;

        glDrawArrays(GL_TRIANGLES, index, trigCount);
    }
}
