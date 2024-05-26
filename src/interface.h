#ifndef INTERFACE
#define INTERFACE

#include "graphics.h"
#include "object.h"
#include "commandLine.h"

struct Interface{
    struct CommandLine *cmd;   
    struct BlockSupervisor *bs;
    struct Graphics *g;
    
    double screenX, screenY;
    bool mouseClick;

    // selection
    vec3 clickVec;
    struct Block *selected;
    BlockPosition blockPos, addBlockPos;
    bool floorIntersection;
};

struct Interface *interfaceInit(struct CommandLine *cmd, struct BlockSupervisor *bs, struct Graphics *g);
void interfaceCursor(mat4 projectionMatrix, mat4 viewMatrix, struct Camera *cam1);

void interfaceAddBlock();

#endif
