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
    BlockId currBlockIndex;

    // selection
    vec3 clickVec;
    struct Block *selected;
    BlockPosition blockPos, addBlockPos;
    bool floorIntersection;
};

struct Interface *interfaceInit(struct CommandLine *cmd, struct BlockSupervisor *bs, struct Graphics *g, struct Text *text);
void interfaceCursor(mat4 projectionMatrix, mat4 viewMatrix, struct Camera *cam1);

void interfaceAddBlock();

void interfaceProcess(struct Interface *in, GLint modelUniformLocation);

#endif
