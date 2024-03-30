#ifndef GRAPHICS
#define GRAPHICS

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define CGLM_DEFINE_PRINTS 1
#define DEBUG 1

#include <cglm/cglm.h>
#include <cglm/types.h>
#include <cglm/io.h>

#include <cglm/mat4.h>
#include <cglm/vec4.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "./gHeader.h"

#include "./shader.h"
#include "./shadersDef.h"
#include "colors.h"

#include "func.h"

#define WINDOW_WIDTH 1362
#define WINDOW_HEIGHT 716

#define MAX_COMMAND_BUFFER_SIZE 200

struct Graphics{
    GLFWwindow *window;
    int width, height;
    float screenRatio;
    float deltaTime, lastFrame;
};

struct Graphics *graphicsInit();

void drawLine(int x1, int y1, int z1, int x2, int y2, int z2);
void drawArrow(vec3 end, float scale);

#endif
