#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define CGLM_DEFINE_PRINTS 1
#define DEBUG 1

#include <cglm/cglm.h>
#include <cglm/types.h>
#include <cglm/io.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "./shaders.h"
#include "./cube.h"

#define WINDOW_WIDTH 1362
#define WINDOW_HEIGHT 716

void graphicsInit();

void drawLine(int x1, int y1, int z1, int x2, int y2, int z2);
