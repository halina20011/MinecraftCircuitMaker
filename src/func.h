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

#include "vector.h"
#include "gHeader.h"

void processInput();

// Callbacks
void keyCallback(GLFWwindow *w, int key, int scancode, int action, int mods);
void characterCallback(GLFWwindow *window, uint32_t codepoint);
void cursorPosCallback(GLFWwindow *w, double x, double y);
void scrollCallback(GLFWwindow *w, double x, double y);
void framebufferSizeCallback(GLFWwindow *w, int width, int height);
void mouseButtonCallback(GLFWwindow *w, int button, int action, int mods);

uint8_t *readFile(const char fileName[], size_t *rSize);

#endif
