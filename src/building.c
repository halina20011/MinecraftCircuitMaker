#include "block.h"

VECTOR_TYPE_FUNCTIONS(struct Building*, BuildingPVector, "");

// # BUILDING FILE FORMAT 
// size of map uint32_t
// # map entry
//  [size of idStr][idStr] id(uint16_t)
// blocks
// number of blocks (uint32_t)
// # block entry
//  id (uint16_t) 
//  abs pos (x, y, z int16_t)
//  rotation (uint8_t)

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
