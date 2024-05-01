#include <stdio.h>
#include <stdlib.h>

#include "./graphics.h"
#include "./blocks.h"
#include "./block.h"
#include "color.h"

#include "func.h"
#include "ui.h"
#include "text.h"

struct Graphics *g;

bool command = false;
char commandBuffer[MAX_COMMAND_BUFFER_SIZE];
size_t commandBufferSize = 0;

vec3 up = {0, 1, 0};
vec3 cameraPos = {0, 0, 3};

vec3 cameraFront = {0, 0, -1};
vec3 cameraUp = {0, 1, 0};

float yaw = -90, pitch = 0;
float fov = 45;

float cube[];
size_t cubeSize;

void defineUi(struct Ui *ui){
    struct UiElement blockHolder = uiElementInit(ui);
    uiAddElement(&blockHolder, &ui->root, ABSOLUTE_PERCENTAGE, PERCENTAGE, 10, 20, 20, 90);
    
    struct UiElement blockHolder2 = uiElementInit(ui);
    blockHolder2.color = (struct ElementColor){255, 0, 0};
    uiAddElement(&blockHolder2, &blockHolder, RELATIVE_PERCENTAGE, PERCENTAGE, 10, 10, 80, 20);
    // the command line
    // uiAddElement(ui, ui->root, ABSOLUTE, );

    uiBake(ui);
}

int main(){
    // printf("%i %i\n", DEBUG, CGLM_DEFINE_PRINTS);
    // printf("UwU\n");
    // loadBlocks(BLOCKS);
    g = graphicsInit();

    struct Ui *ui = uiInit(g->window);
    // defineUi(ui);

    struct Shader *shader = shaderInit(VERTEX_SHADER, FRAGMENT_SHADER);

    GLint posAttrib = glGetAttribLocation(shader->program, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
    GLint textAttrib = glGetAttribLocation(shader->program, "texture");
    glVertexAttribPointer(textAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(posAttrib);
    glEnableVertexAttribArray(textAttrib);

    GLint modelUniformLocation      = getUniformLocation(shader, "model");
    GLint viewUniformLocation       = getUniformLocation(shader, "view");
    GLint projectionUniformLocation = getUniformLocation(shader, "projection");

    GLint textureUniform = getUniformLocation(shader, "textureSampler");
    printf("texture uniform %i\n", textureUniform);
    
    GLuint colorUniform = -1;
    // GLuint colorUniform = getUniformLocation(shader, "color");
    glUniform3f(colorUniform, 0.5, 0.5, 0.5);
    
    // vec3 cubePositions[] = {
    //     {0, 0, 0},
    //     {2, 5, -10},
    //     {1, 2, 3},
    //     {-1, -4, -2},
    //     {-1, 2, 4},
    //     {3, 3, 3},
    //     {-2, -3, 3},
    // };
    vec3 cubePositions[] = {
        {-4, 0, 0},
        {-2, 0, 0},
        {0, 0, 0},
        {2, 0, 0},
        {4, 0, 0},
        {6, 0, 0},
        {8, 0, 0},
        {10, 0, 0},
    };
    
    const size_t cubePositionsSize = sizeof(cubePositions) / sizeof(vec3);

    struct Text *text = NULL;
    // struct Text *text = textInit(&g->screenRatio);
    // text->screenRatio = &g->screenRatio;
    // SET_COLOR(text->colorUniform, RED);

    size_t blocksTypesSize;
    struct BlockType *blocksTypes = loadBlocks(&blocksTypesSize);
    GLuint t = loadAllBlocks("Assets/texture.bin");
    // GLuint t = loadAllBlocks("Assets/texture.data");
    printf("texture %i\n", t);

    useShader(shader);
    glActiveTexture(GL_TEXTURE0 + t);
    // glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t);
    glUniform1i(textureUniform, t);
    // shaderPrint(shader);

    GLint maxTexSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    printf("max texture size %i\n", maxTexSize);

    // int prev = 0, curr;
    // int couter = 0;
    //
    // struct BlockType target = readBlock("Assets/Blocks/target");
    struct BlockType piston = readBlock("Assets/Blocks/pistonDef");

    while(!glfwWindowShouldClose(g->window)){
        float currFrame = glfwGetTime();
        g->deltaTime = currFrame - g->lastFrame;
        g->lastFrame = currFrame;

        // curr = currFrame;
        // if(prev != curr){
        //     prev = curr;
        //     // printf("coutner %i\n", couter);
        //     couter = 1;
        // }
        // else{
        //     couter++;
        // }

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        useShader(shader);

        mat4 view, projection, model;
        glm_mat4_identity(view);
        // glm_mat4_identity(projection);
        glm_mat4_identity(model);

        glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, (float*)projection);
        glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, (float*)view);
        glm_translate(model, (vec3){0.8, 0.8, 0});
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);

        // drawArrow(up, 2);
        // SET_COLOR(colorUniform, GREEN);
        // drawArrow(right, 2);
        // SET_COLOR(colorUniform, BLUE);
        // drawArrow(front, 2);
        vec3 look, cameraRight, currCameraUp;
        glm_vec3_copy(cameraFront, look);
        glm_vec3_normalize(look);
        glm_vec3_cross(look, up, cameraRight);
        glm_vec3_cross(cameraRight, look, currCameraUp);

        // glm_vec3_cross(cameraRight, cameraPos, rightR);
        
        // SET_COLOR(colorUniform, LIGHT_PURPLE);
        // drawArrow(cameraFront, -0.1);
        // SET_COLOR(colorUniform, RED);
        // drawArrow(cameraRight, 0.1);
        // SET_COLOR(colorUniform, BLUE);
        // drawArrow(currCameraUp, 0.1);
        
        // glm_ortho(0, 800.0f, 0, 600.0f, -10.0f, 200.0f, projection);
        glm_perspective(glm_rad(fov), g->screenRatio, 0.1f, 100.0f, projection);
        // printf("fov: %f\n", glm_rad(fov));
        // glm_mat4_print(projection, stderr);
        // return 0;

        vec3 center;
        glm_vec3_add(cameraPos, cameraFront, center);
        glm_lookat(cameraPos, center, cameraUp, view);

        // glm_vec3_print(cameraFront, stdout);
        
        glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, (float*)projection);

        float trig[] = {
            0, 0, 0, 0, 0,
            1, 0, 0, 1, 0,
            0.5, 1, 0, 0.5, 1
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(trig), trig, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // struct BlockType block = blocksTypes[1];

        // struct BlockType block = piston;
        // glm_mat4_identity(model);
        // glm_translate(model, cubePositions[1]);
        // glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);
        // drawBlock(block);

        // printf("================================================\n");
        // return 1;
        for(size_t i = 0; i < blocksTypesSize; i++){
            struct BlockType block = blocksTypes[i];
            printf("%i\n", block.type);
            glm_mat4_identity(model);
            glm_translate(model, cubePositions[i]);
            glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);
            drawBlock(block);
        }
        // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * piston->dataSize, piston->data, GL_STATIC_DRAW);
        // glBufferData(GL_ARRAY_BUFFER, sizeof(trig), trig, GL_STATIC_DRAW);
        // glBufferData(GL_ARRAY_BUFFER, cubeSize, cube, GL_STATIC_DRAW);

        // for(size_t i = 0; i < cubePositionsSize; i++){
        //     glm_mat4_identity(model);
        //     glm_translate(model, cubePositions[i]);
        //
        //     float angle = glm_rad(20 * i);
        //     glm_rotate(model, angle, (vec3){1, 0.3, 0.5});
        //
        //     glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);
        //
        //     vec3 color = {0.5, 0.5, 0.5};
        //     glm_vec3_rotate(color, angle, (vec3){0.4, -1.4, 2});
        //     glUniform3fv(colorUniform, 1, (float*)color);
        //     // glUniform3f(colorUniform, 0.5, 0.5, 0.5);
        //     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //     glDrawArrays(GL_TRIANGLES, 0, 36);
        //
        //     glUniform3f(colorUniform, 0, 0, 0);
        //     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //     glDrawArrays(GL_TRIANGLES, 0, 36);
        // }

        // drawGrid(-1, -1, -1, 4, 4, 4);
        // x axis
        // glUniform3f(colorUniform, 1, 0, 0);
        // drawArrow(1, 0);
        // // y axis
        // glUniform3f(colorUniform, 0, 1, 0);
        // // z axis
        // glUniform3f(colorUniform, 0, 0, 1);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glDisable(GL_DEPTH_TEST);
        // glDepthMask(GL_FALSE);
        // glDepthFunc(GL_ALWAYS);
        // uiDraw(ui);

        // if(command){
        //     textDraw(text, commandBuffer, -1, -1.0f + 0.01f, 0.05);
        // }
        processInput();
        glfwSwapBuffers(g->window);

        glfwPollEvents();
    }

    return 0;
}

float cube[] = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f
};

size_t cubeSize = sizeof(cube);
