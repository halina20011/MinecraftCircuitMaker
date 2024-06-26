#include "interface.h"

extern struct Interface *interface;

struct Interface *interfaceInit(struct CommandLine *cmd, struct BlockSupervisor *bs, struct Graphics *g, struct Text *text){
    struct Interface *in = malloc(sizeof(struct Interface));

    in->cmd = cmd;
    in->bs = bs;
    in->g = g;
    in->text = text;

    in->buffer = malloc(sizeof(char) * (COMMAND_MAX_SIZE + 1));
    in->bufferSize = 0;

    in->activeBuffer = false;
    in->activeCmd = false;
    in->activeUi = false;

    in->blockIsActive = true;
    in->currBlockIndex = 0;
    in->currBuildingIndex = 0;
    in->selectedBlock = NULL;
    in->selectedBuilding = NULL;

    in->mouseClick = false;
    in->rightClick = false;

    in->rotate = 0;
    in->facing = 0;

    return in;
}

void interfaceAddBlock(){
    printf("adding block\n");
    uint16_t blockType = interface->cmd->optionsIndicies[2];
    // vec3 interface->addBlockPos
    addBlock(interface->bs, blockType, interface->addBlockPos, interface->facing);
}

void interfaceExportBuilding(){
    struct CommandLine *cmd = interface->cmd;
    char *path = &interface->buffer[cmd->optionsIndicies[2]];
    printf("exporting as building\n");
    printf("path '%s'\n", path);
    exportAsBuilding(interface->bs, path);
}

void interfaceExportScene(){
    printf("exporting as scene");
}

void interfaceLoadBuilding(){
    struct CommandLine *cmd = interface->cmd;
    char *path = &interface->buffer[cmd->optionsIndicies[2]];
    printf("loading building\n");
    printf("path '%s'\n", path);
    buildingLoad(interface->bs, path);
}

void interfaceAddBuilding(){
    if(interface->bs->buildingTypes->size){
        buildingAdd(interface->bs, interface->bs->buildingTypes->data[interface->currBuildingIndex]->id, interface->addBlockPos, EAST);
    }
}

void interfaceLoadScene(){
    printf("exporting as scene");
}

void interfaceSelectBlock(){
    char *str = interface->buffer;
    interface->activeUi = false;
    printf("block %s\n", str);

    struct BlockSupervisor *bs = interface->bs;
    if(interface->blockIsActive && bs->availableBlockTypesSize){
        for(BlockTypeId i = 0; i < bs->availableBlockTypesSize; i++){
            BlockTypeId id = bs->availableBlockTypes[i];
            struct BlockType *bt = &bs->blockTypes[id];
            if(strncmp(interface->buffer, bt->idStr, interface->bufferSize) == 0){
                interface->currBlockIndex = id;
                return;
            }
        }
    }
    else if(!interface->blockIsActive && bs->buildingTypes->size){
        for(BlockTypeId i = 0; i < bs->buildingTypes->size; i++){
            BlockTypeId id = bs->buildingTypes->data[i]->id;
            struct BuildingType *bt = bs->buildingTypes->data[id];
            if(strncmp(interface->buffer, bt->name, interface->bufferSize) == 0){
                interface->currBlockIndex = id;
                return;
            }
        }

    }
}

void interfaceBuffer(char c){
    if(COMMAND_MAX_SIZE <= interface->bufferSize){
        fprintf(stderr, "command is too long max is %i", COMMAND_MAX_SIZE);
    }
    else{
        interface->buffer[interface->bufferSize++] = c;
        interface->buffer[interface->bufferSize] = '\0';
    }
}

void interfaceProcess(struct Interface *in, GLint modelUniformLocation, GLint colorUniform){
    // draw curr object's name
    char *currName = NULL;
    if(in->blockIsActive && in->bs->availableBlockTypesSize){
        currName = in->bs->blockTypes[in->currBlockIndex].idStr;
    }
    else if(in->bs->buildingTypes->size){
        currName = in->bs->buildingTypes->data[in->currBuildingIndex]->name;
    }

    // draw cursor position
    char posBuffer[101];
    snprintf(posBuffer, 100, "%i %i %i", 
            in->addBlockPos[0], in->addBlockPos[1], in->addBlockPos[2]
    );
    // printf("pos %s\n", posBuffer);
    float textWidth = textGetWidth(posBuffer);
    float textHeight = textSetHeightPx(25);
    // printf("%f %f\n", textWidth, textHeight);
    textDrawOnScreen(in->text, posBuffer, 1.0f - textWidth, -1.0f + textHeight, modelUniformLocation);

    float offset = textSetHeightPx(25);
    if(!currName){
        textDrawOnScreen(in->text, "(zero buildings)", -1, 1.f - offset, modelUniformLocation);
    }
    else{
        textDrawOnScreen(in->text, currName, -1, 1.f - offset, modelUniformLocation);
    }

    char index[2] = {in->facing + '0', 0};
    textDrawOnScreen(in->text, index, -1, 1.f - 2.0f * offset, modelUniformLocation);

    if(interface->activeCmd && !interface->activeUi){
        commandLineDraw(interface->cmd, modelUniformLocation, colorUniform);
        // SET_COLOR(colorUniform, WHITE);
    }

    // int s = strlen(currBlock);
    // printf("curr block %s %i\n", currBlock, s);

    // printf("%i %i\n", interface->facing, interface->rotate);
    if(interface->rotate){
        // printf("%p %p\n", interface->selectedBlock, interface->selectedBuilding);
        if(interface->blockIntersection && interface->selectedBlock){
            interface->selectedBlock->facing = interface->rotate - 1;
        }
        else if(interface->buildingIntersection && interface->selectedBuilding){
            interface->selectedBuilding->facing = interface->rotate - 1;
        }

        interface->rotate = 0;
    }

    // place block if needed
    if(interface->mouseClick){
        if(interface->blockIsActive){
            addBlock(interface->bs, interface->bs->availableBlockTypes[interface->currBlockIndex], interface->addBlockPos, interface->facing);
        }
        else if(interface->bs->buildingTypes->size){
            buildingAdd(interface->bs, interface->bs->buildingTypes->data[interface->currBuildingIndex]->id, interface->addBlockPos, interface->facing);
        }
        interface->mouseClick = false;
    }
    if(interface->rightClick){
        if(interface->blockIntersection){
            blockDelete(in->bs, interface->blockPos);
        }
        else if(interface->buildingIntersection){
            buildingDelete(interface->bs, interface->selectedBuilding);
            interface->selectedBuilding = NULL;
            interface->buildingIntersection = false;
        }
        interface->rightClick = false;
    }
}

void floorIntersection(vec3 point, vec3 direction, BlockPosition *rPos){
    // plane = Ax + By + Cz + D = 0
    // line =>
    //  ___
    //  | x = x0 + at
    //  | y = y0 + bt
    //  | z = z0 + ct
    //  __

    // plane => 0*x + 1*y + 0*z = 0
    // line =>  y = y0 + bt
    //
    //  y0 + bt = 0
    //  bt = -y0
    //  t = -y0 / b

    float x0 = point[0];
    float y0 = point[1];
    float z0 = point[2];

    float a = direction[0] - x0;
    float b = direction[1] - y0;
    float c = direction[2] - z0;

    float t = (-0.5 - y0) / b;

    float x = x0 + a * t;
    // float y = y0 + b * t;
    float z = z0 + c * t;

    // WRONG => *rPos[n] = 1;
    (*rPos)[0] = roundf(x);
    (*rPos)[1] = 0;
    (*rPos)[2] = roundf(z);
    // vec3 position = {roundf(intersection[0]), roundf(intersection[1]) + 1, roundf(intersection[2])};
    // *rPos[1] = y;
    // *rPos[2] = z;

    // printf("(%f %f %f) (%f %f %f) %f (%f %f %f)\n", x0, y0, z0, a, b, c, tx, x, y, z);
    // glm_vec3_print(*rPos, stdout);
}

void interfaceCursor(mat4 projectionMatrix, mat4 viewMatrix, struct Camera *cam1){
    float x = (2.0f * interface->screenX) / (float)interface->g->width - 1.0f;
    float y = 1.0f - (2.0f * interface->screenY) / (float)interface->g->height;
    float z = 1.0f;

    vec3 rayEnd = {x, y, z};

    vec4 rayClip = {rayEnd[0], rayEnd[1], -1.0, 1.0};

    vec4 rayEye = {};
    mat4 projInv = {};
    glm_mat4_inv(projectionMatrix, projInv);
    glm_mat4_mulv(projInv, rayClip, rayEye);

    rayEye[0] = rayEye[0];
    rayEye[1] = rayEye[1];
    rayEye[2] = -1;
    rayEye[3] = 0.0;

    mat4 viewInv = {};
    vec4 rayWorld4 = {};
    glm_mat4_inv(viewMatrix, viewInv);
    glm_mat4_mulv(viewInv, rayEye, rayWorld4);
    // rayWorld = {rayWorld4[0], rayWorld4[1], rayWorld4[2]};
    
    vec3 rayWorld = {rayWorld4[0],rayWorld4[1],rayWorld4[2]};
    
    glm_vec3_normalize(rayWorld);
    // glm_vec3_print(rayWorld, stdout);
    // glm_vec3_scale(clickVec, 2, clickVec);
    glm_vec3_add(cam1->cameraPos, rayWorld, interface->clickVec);

    // for(size_t i = 0; i < intersections->size; i++){
    //     free(intersections->data[i]);
    // }
    // intersections->size = 0;
    
    float minVal = FLT_MAX;
    vec3 minIntersectionPoint = {};
    uint8_t side = 0;

    interface->selectedBlock = NULL;
    interface->selectedBuilding = NULL;

    vec3 direction = {};
    glm_vec3_sub(interface->clickVec, cam1->cameraPos, direction);
    glm_vec3_normalize(direction);

    // intersection with block
    for(size_t i = 0; i < interface->bs->blocks->size; i++){
        struct Block *block = interface->bs->blocks->data[i];
        struct BlockType *blocksType = &interface->bs->blockTypes[block->id];

        float r = 0;
        mat4 mat = {};
        glm_mat4_identity(mat);
        glm_translate(mat, (vec3){block->position[0], block->position[1], block->position[2]});
        uint8_t currSide;
        if(boundingBoxIntersection(&blocksType->boundingBox, cam1->cameraPos, direction, mat, &r, &currSide)){
            // printf("intersection %zu\n", i);
            
            // glm_vec3_scale(direction, r, direction);
            // glm_vec3_add(direction, cam1->cameraPos, direction);
            
            // drawPoint(direction, colorUniform);
            if(r < minVal){
                minVal = r;
                // block->position
                ASSIGN3(minIntersectionPoint, block->position);
                side = currSide;
                interface->selectedBlock = block;
            }

            // float *v = malloc(sizeof(float) * 3);
            // memcpy(v, direction, sizeof(float) * 3);
            // Vec3VectorPush(intersections, v);
        }
    }
    
    float minValBuilding = FLT_MAX;
    vec3 minIntersectionPointBuilding = {};
    // intersection with a building
    for(size_t i = 0; i < interface->bs->buildings->size; i++){
        struct Building *b = interface->bs->buildings->data[i];
        struct BuildingType *bt = interface->bs->buildingTypes->data[b->id];
        // struct Block *block = interface->bs->blocks->data[i];
        // struct BlockType *blocksType = &interface->bs->blockTypes[block->id];

        float r = 0;

        mat4 mat;
        glm_mat4_identity(mat);
        
        glm_rotate_at(mat, blockPosVec3(b->position), glm_rad(facingToRad(b->facing)), (vec3){0, 1, 0});
        glm_translate(mat, blockPosVec3(b->position));

        uint8_t currSide;
        if(boundingBoxIntersection(&bt->boundingBox, cam1->cameraPos, direction, mat, &r, &currSide)){
            if(r < minValBuilding){
                minValBuilding = r;
                // block->position
                ASSIGN3(minIntersectionPointBuilding, b->position);
                side = currSide;
                interface->selectedBuilding = b;
            }
        }
    }
    
    interface->floorIntersection = false;
    interface->buildingIntersection = false;
    interface->blockIntersection = false;

    if(minVal == FLT_MAX && minValBuilding == FLT_MAX){
        floorIntersection(cam1->cameraPos, interface->clickVec, &interface->addBlockPos);
        interface->floorIntersection = true;
    }
    else if(minVal == FLT_MAX){
        interface->buildingIntersection = true;
        ASSIGN3(interface->blockPos, minIntersectionPointBuilding);
    }
    else{
        interface->blockIntersection = true;

        ASSIGN3(interface->blockPos, minIntersectionPoint);
        ASSIGN3(interface->addBlockPos, minIntersectionPoint);

        float offset = 1;\
        if(cam1->cameraPos[side] < interface->addBlockPos[side]){\
            offset = -1;\
        }\
        interface->addBlockPos[side] += offset;\
    }
        // ASSIGN3(intersectionPoint, blockPos);
        // ASSIGN3(addBlockPos, intersectionPoint);
    // else{
    //     ASSIGN3(intersectionPoint, minIntersectionPoint);
    // }

    // for(int i = 0; i < intersections->size; i++){
    //     float *p = intersections->data[i];
    //     drawPoint(p, colorUniform);
    // }
}

void interfaceDraw(GLuint modelUniformLocation, GLuint colorUniform, GLuint arrayBuffer, GLuint elementArrayBuffer){
    // draw block selection box
    mat4 blockMatrix = {};
    glm_mat4_identity(blockMatrix);
    glm_translate(blockMatrix, blockPosVec3(interface->addBlockPos));
    glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)blockMatrix);
    
    if(interface->bs->availableBlockTypesSize){
        struct BlockType *bt = &interface->bs->blockTypes[interface->currBlockIndex];
        struct BoundingBox *bb = &bt->boundingBox;

        if(interface->blockIntersection && interface->selectedBlock){
            SET_COLOR(colorUniform, YELLOW);
            boundingBoxDraw(bb, interface->blockPos, interface->rotate, arrayBuffer, elementArrayBuffer, modelUniformLocation);
        }

        if(interface->blockIsActive){
            SET_COLOR(colorUniform, GREEN);
            boundingBoxDraw(bb, interface->addBlockPos, interface->facing, arrayBuffer, elementArrayBuffer, modelUniformLocation);
        }
    }

    if(interface->bs->buildingTypes->size){
        if(interface->buildingIntersection && interface->selectedBuilding){
            struct Building *b = interface->selectedBuilding;
            struct BuildingType *currBuilding = interface->bs->buildingTypes->data[b->id];
            struct BoundingBox *boundingBoxB = &currBuilding->boundingBox;

            SET_COLOR(colorUniform, YELLOW);
            boundingBoxDraw(boundingBoxB, b->position, b->facing, arrayBuffer, elementArrayBuffer, modelUniformLocation);
        }

        if(!interface->blockIsActive){
            struct BoundingBox *bb = &interface->bs->buildingTypes->data[interface->currBuildingIndex]->boundingBox;
            SET_COLOR(colorUniform, GREEN);
            boundingBoxDraw(bb, interface->addBlockPos, interface->facing, arrayBuffer, elementArrayBuffer, modelUniformLocation);
        }
    }
}
