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
#include "./shaders.h"

#include "color.h"
#include "camera.h"
#include "func.h"

#define WINDOW_WIDTH 1362
#define WINDOW_HEIGHT 716

#define MAX_COMMAND_BUFFER_SIZE 200

#define ASSIGN3(val1, val2) {\
    val1[0] = val2[0];\
    val1[1] = val2[1];\
    val1[2] = val2[2];\
}

struct Graphics{
    GLFWwindow *window;
    int width, height;
    float screenRatio;
    float deltaTime, lastFrame;
    struct Camera **cams;
    size_t camIndex;
    size_t camSize;
    struct Camera *camera;
};

struct Graphics *graphicsInit();
void graphicsAddCameras(struct Graphics *g, struct Camera **cams, size_t size);

void drawLine(float x1, float y1, float z1, float x2, float y2, float z2);
void drawLineVec(vec3 start, vec3 end);
void drawLineDirection(vec3 pos, vec3 relDirection);
void drawDirection(vec3 start, vec3 end, float size);
void drawArrow(vec3 end, float scale);

void drawPoint(vec3 pos, GLint colorUniform);

void *readBitmap(const char fileName[]);
GLuint loadTexture(uint8_t *data, int width, int height);

#endif
