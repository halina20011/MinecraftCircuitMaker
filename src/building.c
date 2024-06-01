#include "object.h"

VECTOR_TYPE_FUNCTIONS(struct BuildingType*, BuildingTypePVector);

void buildingReload(struct Building *building){

}

void buildingDraw(struct BlockSupervisor *bs){
    struct BuildingPVector *buildings = bs->buildings;
    // printf("size %zu\n", buildings->size);
    for(size_t i = 0; i < buildings->size; i++){
        struct Building *b = buildings->data[i];
        struct BuildingType *bt = bs->buildingTypes->data[b->id];

        vec3 buildingPos = {b->position[0], b->position[1], b->position[2]};
        for(size_t j = 0; j < bt->size; j++){
            struct BlockBuilding *bb = bt->blocks[j];
            struct BlockType *blockType = &(bs->blockTypes[bb->id]);

            mat4 model;
            glm_mat4_identity(model);
            
            vec3 blockPosition = {bb->pos[0], bb->pos[1], bb->pos[2]};
            glm_rotate_at(model, buildingPos, glm_rad(facingToRad(b->facing)), (vec3){0, 1, 0});
            glm_translate(model, buildingPos);
            glm_translate(model, blockPosition);

            vec3 rotation = {};
            switch(bb->facing){
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
}

void buildingAdd(struct BlockSupervisor *bs, BuildingTypeId id, BlockPosition pos, BlockRotation facing){
    struct Building *b = malloc(sizeof(struct Building));
    b->id = 0;
    b->index = bs->buildings->size;
    b->position[0] = pos[0];
    b->position[1] = pos[1];
    b->position[2] = pos[2];
    b->facing = facing;
    BuildingPVectorPush(bs->buildings, b);
}

void buildingDelete(struct BlockSupervisor *bs, struct Building *building){
    // swap the building with the last one if needed
    if(bs->buildings->size != 1){
        struct Building *lastBuild = bs->buildings->data[bs->buildings->size - 1];
        lastBuild->index = building->index;
        bs->buildings->data[lastBuild->index] = lastBuild;
    }

    bs->buildings->size--;
    // printf("freeing block %p\n", block);
    free(building);
}

void buildingLoad(struct BlockSupervisor *bs, const char fileName[]){
    FILE *f = fopen(fileName, "r");
    if(!f){
        fprintf(stderr, "failed to open building");
        return;
    }

    struct BuildingType *building = malloc(sizeof(struct BuildingType));

    BuildingTypeId buildingId = bs->buildingTypes->size;
    building->id = buildingId;

    // blocks
    BlockTypesSize blockTypesSize;
    fread(&blockTypesSize, sizeof(BlockTypesSize), 1, f);
    BlockTypeId *blockTypesRemap = malloc(sizeof(BlockTypesSize) * blockTypesSize);

    // read block types
    BlockTypeIdStr idStr[MAX_BLOCK_ID_SIZE + 1];
    BlockTypeIdStrSize idStrSize;
    for(BlocksSize i = 0; i < blockTypesSize; i++){
        fread(&idStrSize, sizeof(BlockTypeIdStrSize), 1, f);
        fread(&idStr, sizeof(BlockTypeIdStr), idStrSize, f);
        idStr[idStrSize] = '\0';

        // block type will have a str id
        // check if the id exists/is valid in the current "version"
        // if the block is not in curr version don't continue loading, exit
        BlockTypeId id = blockId(bs, idStr);
        if(id == BLOCK_NOT_FOUND){
            fprintf(stderr, "failed to load a building, the block type '%s' is invalid\n", idStr);
            free(building);
            return;
        }

        blockTypesRemap[i] = id;
    }

    building->boundingBox.min[0] = FLT_MAX;
    building->boundingBox.min[1] = FLT_MAX;
    building->boundingBox.min[2] = FLT_MAX;

    building->boundingBox.max[0] = FLT_MIN;
    building->boundingBox.max[1] = FLT_MIN;
    building->boundingBox.max[2] = FLT_MIN;

    BlocksSize blocksSize;
    fread(&blocksSize, sizeof(BlocksSize), 1, f);
    BlockTypeId id;
    BlockPosition pos;
    BlockRotation rot;
    struct BlockBuilding **blocks = malloc(sizeof(struct BlockBuilding*) * blocksSize);
    for(BlocksSize i = 0; i < blocksSize; i++){
        fread(&id, sizeof(BlockTypeId), 1, f);
        fread(&pos, sizeof(BlockPosition), 1, f);
        fread(&rot, sizeof(BlockRotation), 1, f);
        printf("block %i id %i, pos %i %i %i, rot %i\n", i, id, pos[0], pos[1], pos[2], rot);
        struct BlockBuilding *bb = malloc(sizeof(struct BlockBuilding));
        bb->id = blockTypesRemap[id];
        bb->facing = rot;
        bb->pos[0] = pos[0];
        bb->pos[1] = pos[1];
        bb->pos[2] = pos[2];
        blocks[i] = bb;

        building->boundingBox.min[0] = MIN(pos[0], building->boundingBox.min[0]);
        building->boundingBox.min[1] = MIN(pos[1], building->boundingBox.min[1]);
        building->boundingBox.min[2] = MIN(pos[2], building->boundingBox.min[2]);
        building->boundingBox.max[0] = MAX(pos[0], building->boundingBox.max[0]);
        building->boundingBox.max[1] = MAX(pos[1], building->boundingBox.max[1]);
        building->boundingBox.max[2] = MAX(pos[2], building->boundingBox.max[2]);
    }

    building->blocks = blocks;
    building->size = blocksSize;

    building->boundingBox.min[0] -= 0.5; 
    building->boundingBox.min[1] -= 0.5; 
    building->boundingBox.min[2] -= 0.5; 

    building->boundingBox.max[0] += 0.5; 
    building->boundingBox.max[1] += 0.5; 
    building->boundingBox.max[2] += 0.5; 
    boundingBoxPrint(&building->boundingBox);

    // save the path of the building file
    BuildingPathSize pathSize;
    fread(&pathSize, sizeof(BuildingPathSize), 1, f);
    BuildingPath path = strdup(fileName);

    char *name = basename((char*)path);
    printf("building name %s\n", name);
    building->name = name;

    building->path = path;

    BuildingTypePVectorPush(bs->buildingTypes, building);
}

void buildingLoadFromDirectory(struct BlockSupervisor *bs, const char directoryPath[]){
    DIR *d = opendir(directoryPath);
    if(!d){
        fprintf(stderr, "failed to open dir: %s\n", directoryPath);
        exit(1);
    }

    struct dirent *dir;
    while((dir = readdir(d)) != NULL){
        if(dir->d_type != DT_DIR){
            char filePath[255];
            strcpy(filePath, directoryPath);
            strcat(filePath, "/");
            strcat(filePath, dir->d_name);
            buildingLoad(bs, filePath);
        }
    }
    closedir(d);
}
