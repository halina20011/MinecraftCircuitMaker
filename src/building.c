#include "object.h"

VECTOR_TYPE_FUNCTIONS(struct Building*, BuildingPVector);

// # BUILDING FILE FORMAT 
// size of blockTypes (uint16_t)
// ----- BLOCKTYPE ENTRY -----
//  size of idStr (uint8_t)]
//  idStr (char*)
//  id (uint16_t)
//
// number of blocks (uint32_t)
// ----- BLOCK ENTRY -----
//  id (uint16_t) 
//  abs pos (x, y, z int16_t)
//  rotation (uint8_t)
//
// size of buildingTypes (uint16_t)
// ----- BUILDING TYPE ENTRY -----
//  building size path (uint8_t)
//  building path (char*)
//
// number of buildings (uint32_t)
// ----- BUILDING ENTRY -----
//  id (uint16_t)
//  rotation (uint8_t)
//  abs pos of origin (x, y, z)

void buildingLoadDirectory(const char directoryPath[]){
    DIR *d = opendir(BLOCKS_DIR_PATH);
    if(!d){
        fprintf(stderr, "failed to open dir: %s\n", BLOCKS_DIR_PATH);
        exit(1);
    }

    struct dirent *dir;
    while((dir = readdir(d)) != NULL){
        if(dir->d_type != DT_DIR){
            char filePath[255];
            strcpy(filePath, BLOCKS_DIR_PATH);
            strcat(filePath, "/");
            strcat(filePath, dir->d_name);

        }
    }
    closedir(d);
}

void buildingExport(struct BlockSupervisor *bS, const char fileName[]){
    
}

void buildingLoad(struct BlockSupervisor *bs, const char fileName[]){
    
}

void blockExport(struct BlockSupervisor *bs, bool unwrapBuildings, const char path[]){
    FILE *f = fopen(path, "wb");
    if(!f){
        fprintf(stderr, "failed to open file %s\n", path);
        return;
    }

    // loop throw all block types and figure out which one are being used
    uint32_t blockTypesSize = 0;
    for(size_t i = 0; i < BLOCK_TYPES_SIZE; i++){
        if(bs->blockTypesHistogram[i]){
            blockTypesSize++;
        }
    }

    uint32_t *blockTypesRemap = malloc(sizeof(uint32_t) * blockTypesSize);
    
    // export the number of blockTypes 
    fwrite(&blockTypesSize, sizeof(uint32_t), 1, f);

    uint16_t blockTypesRemapSize = 0;
    // export each used block type
    for(size_t i = 0; i < BLOCK_TYPES_SIZE; i++){
        if(bs->blockTypesHistogram[i]){
            
            struct BlockType *blockType = &bs->blockTypes[i];
            uint8_t idStrSize = strlen(blockType->idStr);
            // export
            fwrite(&idStrSize, sizeof(uint8_t), 1, f);
            fwrite(&blockType->idStr, sizeof(char), idStrSize, f);
            fwrite(&blockTypesRemapSize, sizeof(uint16_t), 1, f);
            blockTypesRemap[i] = blockTypesRemapSize++;
        }
    }

    // export each block
    uint16_t numberOfBlocks = bs->objects->size;
    fwrite(&numberOfBlocks, sizeof(uint16_t), 1, f);
    for(size_t i = 0; i < bs->objects->size; i++){
        struct Object *b = bs->objects->data[i];
        struct Block *block = b->object;

        // export block
        uint16_t id = blockTypesRemap[block->id];
        fwrite(&id, sizeof(uint16_t), 1, f);
        fwrite(&block->position, sizeof(BlockPosition), 1, f);
        fwrite(&block->facing, sizeof(uint8_t), 1, f);
    }

    fclose(f);
}
