#include "object.h"

#define COMPARE(str, ID)do{\
    if(strcmp(id, str) == 0){\
        return ID;\
    }\
}while(0)

#define MAX(a, b) ((a < b) ? b : a)
#define MIN(a, b) ((a < b) ? a : b)

char **blockNames(){
    char **names = malloc(sizeof(char**) * BLOCK_TYPES_SIZE);
    names[REDSTONE_WIRE] = strdup("redstone_wire");
    names[REDSTONE_TORCH] = strdup("redstone_torch");
    names[PISTON] = strdup("piston");
    names[STICKY_PISTON] = strdup("sticky_piston");
    names[TARGET] = strdup("target");

    return names;
}

BlockTypeId blockId(struct BlockSupervisor *bs, const char *id){
    for(uint16_t i = 0; i < BLOCK_TYPES_SIZE; i++){
        if(strcmp(id, bs->blockTypesNames[i]) == 0){
            return i;
        }
    }
    fprintf(stderr, "block type was not found %s\n", id);
    return BLOCK_NOT_FOUND;
}

void blockTypeBoundingBox(struct BoundingBox *bb, float *data, size_t size){
    bb->min[0] = FLT_MAX;
    bb->min[1] = FLT_MAX;
    bb->min[2] = FLT_MAX;

    bb->max[0] = FLT_MIN;
    bb->max[1] = FLT_MIN;
    bb->max[2] = FLT_MIN;

    for(size_t i = 0; i < size; i += 5){
        bb->min[0] = MIN(data[i + 0], bb->min[0]);
        bb->min[1] = MIN(data[i + 1], bb->min[1]);
        bb->min[2] = MIN(data[i + 2], bb->min[2]);
        
        bb->max[0] = MAX(data[i + 0], bb->max[0]);
        bb->max[1] = MAX(data[i + 1], bb->max[1]);
        bb->max[2] = MAX(data[i + 2], bb->max[2]);
    }
}

bool loadBlock(struct BlockSupervisor *bs, const char fileName[], struct BlockType *block){
    FILE *file = fopen(fileName, "rb");
    if(file == NULL){
        fprintf(stderr, "failed to open file %s\n", fileName);
        exit(1);
    }
    
    BlockTypeIdStrSize idSize = 0;
    fread(&idSize, sizeof(BlockTypeIdStrSize), 1, file);
    // printf("id size %i\n", idSize);

    BlockTypeIdStr idStr[MAX_BLOCK_ID_SIZE + 1];
    fread(&idStr, sizeof(BlockTypeIdStr), idSize, file);
    idStr[idSize] = 0;

    BlockTypeId id = blockId(bs, idStr);
    if(id == BLOCK_NOT_FOUND){
        return false;
    }

    uint32_t size = 0;
    fread(&size, sizeof(uint32_t), 1, file);

    // printf("block size: %i\n", size);
    float *data = malloc(sizeof(float) * size);

    fread(data, sizeof(float), size, file);
    if(feof(file)){
        fprintf(stderr, "blockType file is shorter then the specified size: %s\n", fileName);
        exit(1);
    }

    block->id = id;
    block->idStr = strdup(idStr);

    block->data = data;
    block->dataSize = size;

    // float *d = block->data;
    // for(int i = 0; i < block->dataSize; i += 5){
    //     printf("%f %f %f | %f %f\n", d[i], d[i + 1], d[i + 2], d[i + 3], d[i + 4]);
    // }

    struct BoundingBox *bb = &block->boundingBox;
    blockTypeBoundingBox(bb, data, size);
    // boundingBoxPrint(bb);

    return true;
}

struct BlockType *loadBlocks(struct BlockSupervisor *bs, bool **set, size_t *rDataSize){
    DIR *d = opendir(BLOCKS_DIR_PATH);
    if(!d){
        fprintf(stderr, "failed to open dir: %s\n", BLOCKS_DIR_PATH);
        exit(1);
    }

    struct BlockType *blocks = calloc(BLOCK_TYPES_SIZE, sizeof(struct BlockType));

    *set = calloc(BLOCK_TYPES_SIZE, sizeof(bool));
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
            if(loadBlock(bs, filePath, &blockType)){
                // printf("new block %s: %i\n", blockType.idStr, blockType.id);
                BlockTypeId blockId = blockType.id;
                // printf("block id: %i\n", blockId);
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

GLuint loadBlocksTexture(const char textureFile[]){
    size_t size;
    uint8_t *data = readFile(textureFile, &size);
    // printf("texture size %zu\n", size);
    uint16_t w, h;

    memcpy(&w, data, sizeof(uint16_t));
    memcpy(&h, data + sizeof(uint16_t), sizeof(uint16_t));
    
    // printf("texture %ux%u\n", w, h);

    return loadTexture(data + sizeof(uint16_t) * 2, w, h);
}
