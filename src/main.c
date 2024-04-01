#include <stdio.h>
#include <stdlib.h>

#include "./graphics.h"
#include "./blocks.h"
#include "./block.h"

#include "func.h"
#include "ui.h"
#include "text.h"

#define COLOR(val) glUniform3fv(colorUniform, 1, (float*)val)

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
    struct UiElement *blockHolder = uiElementInit(ui);
    uiAddElement(blockHolder, ui->root, ABSOLUTE_PERCENTAGE, PERCENTAGE, 10, 20, 20, 90);
    
    struct UiElement *blockHolder2 = uiElementInit(ui);
    blockHolder2->color = (struct ElementColor){255, 0, 0};
    uiAddElement(blockHolder2, blockHolder, RELATIVE_PERCENTAGE, PERCENTAGE, 10, 10, 80, 20);
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
    defineUi(ui);

    struct Shader *shader = shaderInit(VERTEX_SHADER, FRAGMENT_SHADER);

    GLint posAttrib = glGetAttribLocation(shader->program, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);

    GLint modelUniformLocation      = getUniformLocation(shader, "model");
    GLint viewUniformLocation       = getUniformLocation(shader, "view");
    GLint projectionUniformLocation = getUniformLocation(shader, "projection");
    
    GLuint colorUniform = getUniformLocation(shader, "color");
    glUniform3f(colorUniform, 0.5, 0.5, 0.5);
    
    vec3 cubePositions[] = {
        {0, 0, 0},
        {2, 5, -10},
        {1, 2, 3},
        {-1, -4, -2},
        {-1, 2, 4},
        {3, 3, 3},
        {-2, -3, 3},
    };
    
    const size_t cubePositionsSize = sizeof(cubePositions) / sizeof(vec3);

    struct Text *text = textInit();
    textColor(text, (vec3){1, 0, 0});

    // shaderPrint(shader);

    // int prev = 0, curr;
    // int couter = 0;
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
        glm_mat4_identity(projection);
        glm_mat4_identity(model);

        glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, (float*)projection);
        glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, (float*)view);
        glm_translate(model, (vec3){0.8, 0.8, 0});
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);

        // drawArrow(up, 2);
        // COLOR(GREEN);
        // drawArrow(right, 2);
        // COLOR(BLUE);
        // drawArrow(front, 2);
        vec3 look, cameraRight, currCameraUp;
        glm_vec3_copy(cameraFront, look);
        glm_vec3_normalize(look);
        glm_vec3_cross(look, up, cameraRight);
        glm_vec3_cross(cameraRight, look, currCameraUp);

        // glm_vec3_cross(cameraRight, cameraPos, rightR);
        COLOR(LIGHT_PURPLE);
        drawArrow(cameraFront, -0.1);
        COLOR(RED);
        drawArrow(cameraRight, 0.1);
        COLOR(BLUE);
        drawArrow(currCameraUp, 0.1);
        
        // glm_ortho(0, 800.0f, 0, 600.0f, -10.0f, 200.0f, projection);
        glm_perspective(glm_rad(fov), g->screenRatio, 0.1f, 100.0f, projection);

        vec3 center;
        glm_vec3_add(cameraPos, cameraFront, center);
        glm_lookat(cameraPos, center, cameraUp, view);

        // glm_vec3_print(cameraFront, stdout);
        
        glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, (float*)projection);

        glBufferData(GL_ARRAY_BUFFER, cubeSize, cube, GL_STATIC_DRAW);

        for(size_t i = 0; i < cubePositionsSize; i++){
            glm_mat4_identity(model);
            glm_translate(model, cubePositions[i]);

            float angle = glm_rad(20 * i);
            glm_rotate(model, angle, (vec3){1, 0.3, 0.5});

            glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);

            vec3 color = {0.5, 0.5, 0.5};
            glm_vec3_rotate(color, angle, (vec3){0.4, -1.4, 2});
            glUniform3fv(colorUniform, 1, (float*)color);
            // glUniform3f(colorUniform, 0.5, 0.5, 0.5);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            glUniform3f(colorUniform, 0, 0, 0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // drawGrid(-1, -1, -1, 4, 4, 4);
        // x axis
        // glUniform3f(colorUniform, 1, 0, 0);
        // drawArrow(1, 0);
        // // y axis
        // glUniform3f(colorUniform, 0, 1, 0);
        // // z axis
        // glUniform3f(colorUniform, 0, 0, 1);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        textDraw(text, "UwU >_<", 0, 0, 0.2);
        glDisable(GL_DEPTH_TEST);
        // glDepthMask(GL_FALSE);
        // glDepthFunc(GL_ALWAYS);
        uiDraw(ui);
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
