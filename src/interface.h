#ifndef INTERFACE
#define INTERFACE

#include "graphics.h"
#include "object.h"
#include "commandLine.h"
#include "text.h"

struct Interface{
    struct CommandLine *cmd;   
    struct BlockSupervisor *bs;
    struct Graphics *g;
    struct Text *text;
    
    double screenX, screenY;
    bool mouseClick, rightClick;

    // block
    bool blockIsActive;
    BlockTypeId currBlockIndex;
    BuildingTypeId currBuildingIndex;

    // selection
    vec3 clickVec;
    struct Block *selectedBlock;
    struct Building *selectedBuilding;
    BlockPosition blockPos, addBlockPos;
    bool floorIntersection, buildingIntersection, blockIntersection;

    uint8_t rotate, facing;
};

struct Interface *interfaceInit(struct CommandLine *cmd, struct BlockSupervisor *bs, struct Graphics *g, struct Text *text);
void interfaceCursor(mat4 projectionMatrix, mat4 viewMatrix, struct Camera *cam1);
void interfaceDraw(GLuint modelUniformLocation, GLuint colorUniform, GLuint arrayBuffer, GLuint elementArrayBuffer);

void interfaceAddBlock();
void interfaceAddBuilding();
void interfaceExportBuilding();
void interfaceExportScene();
void interfaceLoadBuilding();
void interfaceLoadScene();

void interfaceProcess(struct Interface *in, GLint modelUniformLocation);

#endif
