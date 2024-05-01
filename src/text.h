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
#include "./shaders.h"

#define PRINTABLE_CHARACTERS 95

#define TEXTURE_SIZE 128

#define BITMAP_WIDTH 128
#define BITMAP_HEIGHT (BITMAP_WIDTH * PRINTABLE_CHARACTERS)
#define BITMAP_COLOR 1
#define BITMAP_SIZE (BITMAP_WIDTH * BITMAP_HEIGHT * BITMAP_COLOR)

struct CharInfo{
    GLuint texture;
    uint8_t bearingLeft, bearingTop, width;
};

struct Text{
    struct Shader *shader;
    GLuint textureUniform, colorUniform;
    struct CharInfo asciiMap[127];
    float *screenRatio;
};

struct Text *textInit(float *screenRatio);

void textColor(struct Text *text, struct Color color);
void textDraw(struct Text *text, char *str, float x, float y, float size);

#endif
