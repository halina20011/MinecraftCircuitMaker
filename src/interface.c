#include "interface.h"

extern struct Interface *interface;

struct Interface *interfaceInit(struct CommandLine *cmd, struct BlockSupervisor *bs, struct Graphics *g, struct Text *text){
    struct Interface *in = malloc(sizeof(struct Interface));

    in->cmd = cmd;
    in->bs = bs;
    in->g = g;
    in->text = text;

    in->currBlockIndex = 0;
    in->mouseClick = false;
    in->rightClick = false;

    return in;
}

void interfaceAddBlock(){
    printf("adding block\n");
    uint16_t blockType = interface->cmd->optionsIndicies[2];
    // vec3 interface->addBlockPos
    addBlock(interface->bs, blockType, interface->addBlockPos, EAST);
}

void interfaceExportBuilding(){
    struct CommandLine *cmd = interface->cmd;
    char *path = &cmd->command[cmd->optionsIndicies[2]];
    printf("exporting as building\n");
    printf("path '%s'\n", path);
    exportAsBuilding(interface->bs, path);
}

void interfaceExportScene(){
    printf("exporting as scene");
}

void interfaceLoadBuilding(){
    struct CommandLine *cmd = interface->cmd;
    char *path = &cmd->command[cmd->optionsIndicies[2]];
    printf("loading building\n");
    printf("path '%s'\n", path);
    buildingLoad(interface->bs, path);
}

void interfaceLoadScene(){
    printf("exporting as scene");
}

void interfaceProcess(struct Interface *in, GLint modelUniformLocation){
    // draw selected block
    char *currBlock = in->bs->blockTypes[in->currBlockIndex].idStr;
    // int s = strlen(currBlock);
    // printf("curr block %s %i\n", currBlock, s);

    textDrawOnScreen(in->text, currBlock, -1, 1.f - 0.05f, modelUniformLocation);

    // place block if needef
    if(interface->mouseClick){
        addBlock(interface->bs, interface->bs->availableBlockTypes[interface->currBlockIndex], interface->addBlockPos, EAST);
        interface->mouseClick = false;
    }
    if(interface->rightClick){
        // printf("right click\n");
        if(!interface->floorIntersection){
            deleteBlock(in->bs, interface->blockPos);
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

    for(size_t i = 0; i < interface->bs->blocks->size; i++){
        struct Block *block = interface->bs->blocks->data[i];
        struct BlockType *blocksType = &interface->bs->blockTypes[block->id];
        float r = 0;
        vec3 direction = {};
        glm_vec3_sub(interface->clickVec, cam1->cameraPos, direction);
        glm_vec3_normalize(direction);
        mat4 mat = {};
        glm_mat4_identity(mat);
        glm_translate(mat, (vec3){block->position[0], block->position[1], block->position[2]});
        uint8_t currSide;
        if(blockIntersection(blocksType, cam1->cameraPos, direction, mat, &r, &currSide)){
            // printf("intersection %zu\n", i);
            glm_vec3_scale(direction, r, direction);
            glm_vec3_add(direction, cam1->cameraPos, direction);
            // drawPoint(direction, colorUniform);
            if(r < minVal){
                minVal = r;
                // block->position
                ASSIGN3(minIntersectionPoint, block->position);
                // ASSIGN3(minIntersectionPoint, direction);
                side = currSide;
                interface->selected = block;
            }

            // float *v = malloc(sizeof(float) * 3);
            // memcpy(v, direction, sizeof(float) * 3);
            // Vec3VectorPush(intersections, v);
        }
    }
    
    interface->floorIntersection = false;
    if(minVal == FLT_MAX){
        floorIntersection(cam1->cameraPos, interface->clickVec, &interface->addBlockPos);
        interface->floorIntersection = true;
    }
    else{
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
