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

bool mouseClick = false;
double xPos, yPos;

float cube[];
size_t cubeSize;

vec3 clickVec = {0, 0, -1};

vec3 rayWorld;
vec3 floorPos = {};

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

void intersectionDown(vec3 point, vec3 direction, vec3 *rPos){
    // plane = Ax + By + Cz + D = 0
    // line =>
    //  ___
    //  | x = x0 + at
    //  | y = y0 + bt
    //  | z = z0 + ct
    //  __

    // plane => 0*x + 1*y + 0*z = 0
    // line =>  y = y0 + bt
    //
    //  y0 + bt = 0
    //  bt = -y0
    //  t = -y0 / b

    float x0 = point[0];
    float y0 = point[1];
    float z0 = point[2];

    float a = direction[0] - x0;
    float b = direction[1] - y0;
    float c = direction[2] - z0;

    float t = (-0.5 - y0) / b;

    float x = x0 + a * t;
    float y = y0 + b * t;
    float z = z0 + c * t;

    // WRONG => *rPos[n] = 1;
    (*rPos)[0] = x;
    (*rPos)[1] = y;
    (*rPos)[2] = z;
    // *rPos[1] = y;
    // *rPos[2] = z;

    // printf("(%f %f %f) (%f %f %f) %f (%f %f %f)\n", x0, y0, z0, a, b, c, tx, x, y, z);
    // glm_vec3_print(*rPos, stdout);
}

int main(){
    // printf("%i %i\n", DEBUG, CGLM_DEFINE_PRINTS);
    // printf("UwU\n");
    // loadBlocks(BLOCKS);
    struct Camera cam1 = {};
    struct Camera cam2 = {};
    struct Camera *cams[2] = {&cam1, &cam2};
    
    cameraInit(&cam1);
    cameraInit(&cam2);

    g = graphicsInit();
    graphicsAddCameras(g, cams, 2);

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
    GLint colorUniform = getUniformLocation(shader, "color");
    printf("texture uniform %i\n", textureUniform);
    
    vec3 cubePositions[] = {
        {-4, 0, -1},
        {-2, 0, -2},
        {0, 0, -2},
        {2, 0, -2},
        {4, 0, -2},
        {6, 0, -2},
        {8, 0, -2},
        {10, 0, -2},
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

        vec3 look, cameraRight, currCameraUp;
        glm_vec3_copy(g->camera->cameraFront, look);
        glm_vec3_normalize(look);
        glm_vec3_cross(look, g->camera->up, cameraRight);
        glm_vec3_cross(cameraRight, look, currCameraUp);
        
        glm_translate(model, (vec3){0.8, 0.8, 0});
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);

        // glm_vec3_cross(cameraRight, cameraPos, rightR);

        SET_COLOR(colorUniform, LIGHT_PURPLE);
        drawArrow(g->camera->cameraFront, 0.1);
        SET_COLOR(colorUniform, RED);
        drawArrow(cameraRight, 0.1);
        SET_COLOR(colorUniform, BLUE);
        drawArrow(currCameraUp, 0.1);
        glm_translate(model, (vec3){-0.8, -0.8, 0});
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);
        
        // glm_ortho(0, 800.0f, 0, 600.0f, -10.0f, 200.0f, projection);
        glm_perspective(glm_rad(g->camera->fov), g->screenRatio, 0.1f, 100.0f, projection);
        // glm_mat4_print(projection, stderr);
        
        vec3 pointZero = {0, 0, 0};
        vec3 cameraDirection = {};
        glm_vec3_sub(g->camera->cameraPos, pointZero, cameraDirection);

        vec3 center;
        glm_vec3_add(g->camera->cameraPos, g->camera->cameraFront, center);
        glm_lookat(g->camera->cameraPos, center, g->camera->cameraUp, view);
        
        glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, (float*)projection);
        
        // draw origin
        // drawPoint(pointZero, colorUniform);
        
        // glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
        // glDrawArrays(GL_LINES, 0, 2);
        
        // glm_vec3_print(pos, stdout);
        // glm_vec3_print(cameraDirection, stdout);
    
        float yawRad = glm_rad(cam1.yaw);
        float pitchRad = glm_rad(cam1.pitch);
        float xOffset = cos(yawRad) * cos(pitchRad);
        float yOffset = sin(pitchRad);
        float zOffset = sin(yawRad) * cos(pitchRad);

        vec3 offset = {xOffset, yOffset, zOffset};
        
        vec3 cam1Center;
        glm_vec3_add(cam1.cameraPos, cam1.cameraFront, cam1Center);
        vec3 cOffset = {};

        // glm_vec3_negate(nCameraFront);
        glm_vec3_scale(offset, -2, offset);
        glm_vec3_add(cam1Center, offset, cOffset);
        SET_COLOR(colorUniform, BLUE);
        // drawLineVec(cam1Center, cOffset);
        // drawPoint(cOffset, colorUniform);

        // drawPoint(center, colorUniform);
        // drawPoint(cOffset, colorUniform);
        // glm_vec3_negate(cOffset);
        // drawLineDirection(cam1Center, cOffset);
        // drawPoint(center, colorUniform);
        
        // float angle = glm_rad(20 * i);
        // glm_rotate(model, angle, (vec3){1, 0.3, 0.5});
        // glm_rotate_x();
        
        if(mouseClick && g->camIndex == 0){
            mouseClick = false;
            printf("%f %f\n", xPos, yPos);
            float x = (2.0f * xPos) / (float)g->width - 1.0f;
            float y = 1.0f - (2.0f * yPos) / (float)g->height;
            float z = 1.0f;

            vec3 rayEnd = {x, y, z};

            vec4 rayClip = {rayEnd[0], rayEnd[1], -1.0, 1.0};

            vec4 rayEye = {};
            mat4 projInv = {};
            glm_mat4_inv(projection, projInv);
            glm_mat4_mulv(projInv, rayClip, rayEye);

            rayEye[0] = rayEye[0];
            rayEye[1] = rayEye[1];
            rayEye[2] = -1;
            rayEye[3] = 0.0;

            mat4 viewInv = {};
            vec4 rayWorld4 = {};
            glm_mat4_inv(view, viewInv);
            glm_mat4_mulv(viewInv, rayEye, rayWorld4);
            // rayWorld = {rayWorld4[0], rayWorld4[1], rayWorld4[2]};
            
            rayWorld[0] = rayWorld4[0];
            rayWorld[1] = rayWorld4[1];
            rayWorld[2] = rayWorld4[2];
            
            glm_vec3_normalize(rayWorld);
            // glm_vec3_print(rayWorld, stdout);
            // glm_vec3_scale(clickVec, 2, clickVec);
            glm_vec3_add(g->camera->cameraPos, rayWorld, clickVec);
            intersectionDown(cam1.cameraPos, clickVec, &floorPos);
        }

        drawPoint(clickVec, colorUniform);
        SET_COLOR(colorUniform, LIGHT_PURPLE);
        drawDirection(cam1.cameraPos, clickVec, 10);

        drawPoint(floorPos, colorUniform);
        vec3 blockPosClick = {roundf(floorPos[0]), roundf(floorPos[1]) + 1, roundf(floorPos[2])};
        mat4 blockMatrix = {};
        glm_mat4_identity(blockMatrix);
        glm_translate(blockMatrix, blockPosClick);
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)blockMatrix);
        
        glBufferData(GL_ARRAY_BUFFER, cubeSize, cube, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, 36);
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);

        // SET_COLOR(colorUniform, LIGHT_PURPLE);
        // drawLineVec(center, cameraFront);
        // SET_COLOR(colorUniform, BLUE);
        // drawLineVec(pointZero, center);
        // drawPoint(lookDir, colorUniform);
        // drawPoint(center, colorUniform);
        // drawPoint(cameraDirection, colorUniform);
        // drawPoint(cameraPos, colorUniform);
        
        // glm_vec3_print(g->camera->cameraPos, stdout);

        // drawPoint(cameraPos, colorUniform);
        // glm_vec3_print(cameraDirection, stdout);
        // glm_vec3_print(pos, stdout);
        // float line2[] = {
        //     x0, y0, z0, 0, 0,
        //     a, b, c, 0, 0
        //     // x, y, z, 0, 0,
        //     // cameraDirection[0], cameraDirection[1], cameraDirection[2], 0, 0,
        // };
        // glBufferData(GL_ARRAY_BUFFER, sizeof(line2), line2, GL_DYNAMIC_DRAW);
        // glDrawArrays(GL_LINES, 0, 2);

        // struct BlockType block = blocksTypes[1];

        SET_COLOR(colorUniform, GRAY);
        float trig[] = {
            -10, -0.5, -10, 0, 0,
            -10, -0.5, 10, 0, 0,
            10, -0.5, 10, 0, 0,
            10, -0.5, -10, 0, 0
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(trig), trig, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // draw camera view ///////////////////////////////////////////////////////
        mat4 cameraMatrix;
        glm_mat4_identity(cameraMatrix);
        glm_lookat(cam1.cameraPos, cam1Center, cam1.cameraUp, cameraMatrix);
        glm_mat4_inv(cameraMatrix, cameraMatrix);
        glm_translate(cameraMatrix, (vec3){0, 0, 1});
        glm_scale(cameraMatrix, (vec3){0.1, 0.1, 0.1});

        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)cameraMatrix);

        SET_COLOR(colorUniform, WHITE);
        float near[] = {
            -1, -1, 0, 0, 0,
            1, -1, 0, 0, 0,
            1, 1, 0, 0, 0,
            -1, 1, 0, 0, 0,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(near), near, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        // SET_COLOR(colorUniform, RED);
        // drawLineVec(cam1Center, pointZero);
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);
        ////////////////////////////////////////////////////////////////////////////

        glUniform4f(colorUniform, 0.5, 0.5, 0.5, 0);
        for(size_t i = 0; i < blocksTypesSize; i++){
            struct BlockType block = blocksTypes[i];
            // printf("%i\n", block.type);
            glm_mat4_identity(model);
            // vec3 pos = {x, y, z};
            // glm_translate(model, pos);
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
        // y axis
        // glUniform3f(colorUniform, 0, 1, 0);
        // z axis
        // glUniform3f(colorUniform, 0, 0, 1);
        
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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
    -0.5f, -0.5f, -0.5f, 0, 0,
     0.5f, -0.5f, -0.5f, 0, 0,
     0.5f,  0.5f, -0.5f, 0, 0,
     0.5f,  0.5f, -0.5f, 0, 0,
    -0.5f,  0.5f, -0.5f, 0, 0,
    -0.5f, -0.5f, -0.5f, 0, 0,

    -0.5f, -0.5f,  0.5f, 0, 0,
     0.5f, -0.5f,  0.5f, 0, 0,
     0.5f,  0.5f,  0.5f, 0, 0,
     0.5f,  0.5f,  0.5f, 0, 0,
    -0.5f,  0.5f,  0.5f, 0, 0,
    -0.5f, -0.5f,  0.5f, 0, 0,

    -0.5f,  0.5f,  0.5f, 0, 0,
    -0.5f,  0.5f, -0.5f, 0, 0,
    -0.5f, -0.5f, -0.5f, 0, 0,
    -0.5f, -0.5f, -0.5f, 0, 0,
    -0.5f, -0.5f,  0.5f, 0, 0,
    -0.5f,  0.5f,  0.5f, 0, 0,

     0.5f,  0.5f,  0.5f, 0, 0,
     0.5f,  0.5f, -0.5f, 0, 0,
     0.5f, -0.5f, -0.5f, 0, 0,
     0.5f, -0.5f, -0.5f, 0, 0,
     0.5f, -0.5f,  0.5f, 0, 0,
     0.5f,  0.5f,  0.5f, 0, 0,

    -0.5f, -0.5f, -0.5f, 0, 0,
     0.5f, -0.5f, -0.5f, 0, 0,
     0.5f, -0.5f,  0.5f, 0, 0,
     0.5f, -0.5f,  0.5f, 0, 0,
    -0.5f, -0.5f,  0.5f, 0, 0,
    -0.5f, -0.5f, -0.5f, 0, 0,

    -0.5f,  0.5f, -0.5f, 0, 0,
     0.5f,  0.5f, -0.5f, 0, 0,
     0.5f,  0.5f,  0.5f, 0, 0,
     0.5f,  0.5f,  0.5f, 0, 0,
    -0.5f,  0.5f,  0.5f, 0, 0,
    -0.5f,  0.5f, -0.5f, 0, 0
};

size_t cubeSize = sizeof(cube);
