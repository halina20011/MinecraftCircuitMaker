#include <stdio.h>
#include <stdlib.h>

#include "./graphics.h"
#include "./object.h"
#include "color.h"
#include "commandLine.h"
#include "interface.h"

#include "func.h"
#include "ui.h"
#include "text.h"

struct Graphics *g;
struct CommandLine *cmd;
struct Text *text;
struct Interface *interface;

bool mouseClick = false;
double xPos, yPos;

float cube[];
size_t cubeSize;

bool placeBlock = false;

NEW_VECTOR_TYPE(float*, Vec3Vector);
VECTOR_TYPE_FUNCTIONS(float*, Vec3Vector);

struct Option *optionsInit(){
    char **blockTypeNames = blockNames();

    struct Option *options = optionNew("", NULL, 2,
        optionNew("use", NULL, 1, optionList(blockTypeNames, BLOCK_TYPES_SIZE, interfaceAddBlock, 0)),
        optionNew("export", NULL, 1, optionNew("block", NULL, 0))
    );

    return options;
}

void defineUi(struct Ui *ui){
    struct UiElement *blockHolder = uiElementInit(ui);
    uiAddElement(blockHolder, ui->root, ABSOLUTE_PERCENTAGE, PERCENTAGE, 75, 5, 20, 90);
    blockHolder->color = (struct ElementColor){40, 40, 40};
    // printf("blockHolder %p %p\n", &blockHolder, blockHolder.parent);
    // printf("%p\n", ui->uiElements->data[1].parent);
    
    struct UiElement *blockHolder2 = uiElementInit(ui);
    blockHolder2->color = (struct ElementColor){255, 0, 0};
    uiAddElement(blockHolder2, blockHolder, RELATIVE_PERCENTAGE, PERCENTAGE, 50, 0, 80, 20);
    // the command line
    // uiAddElement(ui, ui->root, ABSOLUTE, );

    uiBake(ui);
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

    struct Option *options = optionsInit();
    optionPrint(options, 0);
    cmd = commandLineInit(options);

    struct BlockSupervisor *blockSupervisor= blockSupervisorInit();

    interface = interfaceInit(cmd, blockSupervisor, g);

    struct Ui *ui = uiInit(g->window);
    defineUi(ui);

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

    text = textInit(shader, &g->screenRatio);
    useShader(shader);

    addBlock(blockSupervisor, PISTON, (BlockPosition){-5, 0, 0}, EAST);
    addBlock(blockSupervisor, PISTON, (BlockPosition){-3, 0, 0}, SOUTH);
    addBlock(blockSupervisor, PISTON, (BlockPosition){-1, 0, 0}, WEST);
    addBlock(blockSupervisor, PISTON, (BlockPosition){1, 0, 0}, NORTH);
    addBlock(blockSupervisor, PISTON, (BlockPosition){3, 0, 0}, UP);
    addBlock(blockSupervisor, PISTON, (BlockPosition){5, 0, 0}, DOWN);

    GLint maxTexSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    printf("max texture size %i\n", maxTexSize);

    // struct Vec3Vector *intersections = Vec3VectorInit();

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

        useShader(shader);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 view, projection, model;
        glm_mat4_identity(view);
        glm_mat4_identity(projection);
        glm_mat4_identity(model);

        glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, (float*)projection);
        glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, (float*)view);

        // textDrawOnScreen(text, "UwU", -1, -1, modelUniformLocation);
        if(cmd->active){
            commandLineDraw(cmd, modelUniformLocation, colorUniform);
            // SET_COLOR(colorUniform, WHITE);
        }
        useShader(shader);

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
        
        if(g->camIndex == 0){
            interfaceCursor(projection, view, &cam1);
        }

        drawPoint(interface->clickVec, colorUniform);
        SET_COLOR(colorUniform, LIGHT_PURPLE);
        drawDirection(cam1.cameraPos, interface->clickVec, 10);

        // draw selection box
        // drawPoint(interface->intersectionPoint, colorUniform);
        // vec3 blockPos = {roundf(intersectionPoint[0]), roundf(intersectionPoint[1]) + 1, roundf(intersectionPoint[2])};
        
        // draw block selection box
        mat4 blockMatrix = {};
        glm_mat4_identity(blockMatrix);
        glm_translate(blockMatrix, blockPosVec3(interface->addBlockPos));

        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)blockMatrix);

        glBufferData(GL_ARRAY_BUFFER, cubeSize, cube, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, 36);

        // draw add selection box
        if(!interface->floorIntersection){
            mat4 blockMatrix2;
            SET_COLOR(colorUniform, BLUE);
            glm_mat4_identity(blockMatrix2);
            glm_translate(blockMatrix2, blockPosVec3(interface->blockPos));

            glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)blockMatrix2);
            
            glDrawArrays(GL_LINE_LOOP, 0, 36);
        }

        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);

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
        // for(size_t i = 0; i < BLOCK_TYPES_SIZE; i++){
        //     struct BlockType block = blockSupervisor.blockTypes[i];
        //     // printf("%i\n", block.type);
        //     glm_mat4_identity(model);
        //     vec3 pos = {i * 2, 0, -2};
        //     glm_translate(model, pos);
        //     // glm_translate(model, cubePositions[i]);
        //     glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);
        //     drawBlock(block);
        // }

        drawChunks(blockSupervisor);
        drawBlocks(blockSupervisor, modelUniformLocation, textureUniform);
        
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // drawBlocks(&blockSupervisor, modelUniformLocation);

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
