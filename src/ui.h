#ifndef UI
#define UI

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

#include "./shader.h"
#include "./shadersDef.h"
#include "./vector.h"

// 6 float per vertex
//  pos 2
//  color 4
// 4 vertexs
#define ELEMENT_DATE_SIZE 24

#define ELEMENT_HIDDEN 1

// pos type
enum{
    RELATIVE,
    ABSOLUTE
};

// size type
enum{
    PX,
    PERCENTAGE,
    VW,
    VH,
};

#define N_COLOR(v) ((float)v / 255.0f)

struct ElementColor{
    uint8_t r, g, b;
};

struct Ui{
    GLFWwindow *window;
    int width, height;

    struct Vector *uiElements;

    struct UiElement *root;

    size_t idCounter;
    size_t visibleElements;

    float *data;
    size_t bakeSize;
    GLuint vao, vbo;
    struct Shader *shader;
};

struct UiElement{
    size_t id;
    struct UiElement *parent;
    struct UiElement **children;
    size_t childrenSize;

    uint8_t sizeType;
    uint8_t posType;

    // user input
    float iX, iY;
    float iWidth, iHeight;

    // calculated
    float x, y;
    float width, height;
 
    struct ElementColor color;

    uint8_t flags;
};

struct Ui *uiInit(GLFWwindow *w);

void uiBake(struct Ui *ui);

struct UiElement *uiElementInit(struct Ui *ui);
struct UiElement *uiAddElement(struct UiElement *element, struct UiElement *parent, uint8_t posType, uint8_t sizeType, float x, float y, float width, float height);
void uiDraw(struct Ui *ui);

#endif
