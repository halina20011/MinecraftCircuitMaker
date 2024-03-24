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

#include "./shaders.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

void graphicsInit();
