#ifndef TEXT
#define TEXT

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/vec3.h>

#include "./shader.h"
#include "./shadersDef.h"

#define PRINTABLE_CHARACTERS 95

#define TEXTURE_SIZE 128

#define BITMAP_WIDTH 128
#define BITMAP_HEIGHT (BITMAP_WIDTH * PRINTABLE_CHARACTERS)
#define BITMAP_COLOR 1
#define BITMAP_SIZE (BITMAP_WIDTH * BITMAP_HEIGHT * BITMAP_COLOR)

struct Text{
    struct Shader *shader;
    GLuint textureUniform, colorUniform;
    GLuint asciiMap[127];
};

struct Text *textInit();
void textColor(struct Text *text, vec3 color);
void textDraw(struct Text *text, char *str, float x, float y, float size);

#endif
