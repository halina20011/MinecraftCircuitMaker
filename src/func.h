#ifndef FUNC
#define FUNC

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

void processInput();

// Callbacks
void keyCallback(GLFWwindow *w, int key, int scancode, int action, int mods);
void cursorPosCallback(GLFWwindow *w, double x, double y);
void scrollCallback(GLFWwindow *w, double x, double y);
void framebufferSizeCallback(GLFWwindow *w, int width, int height);

#endif
