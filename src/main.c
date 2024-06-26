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

bool placeBlock = false;

NEW_VECTOR_TYPE(float*, Vec3Vector);
VECTOR_TYPE_FUNCTIONS(float*, Vec3Vector);

struct Option *optionsInit(char **blockTypeNames){
    struct Option *options = optionNew("", NULL, 4,
        optionNew("use", NULL, 1, 
            optionList(blockTypeNames, BLOCK_TYPES_SIZE, interfaceAddBlock, 0)
        ),
        optionNew("export", NULL, 2, 
            optionNew("building", NULL, 1, 
                optionArgument(interfaceExportBuilding, 0)
            ), 
            optionNew("scene", NULL, 1, 
                optionArgument(interfaceExportScene, 0)
            )
        ),
        optionNew("load", NULL, 2, 
            optionNew("building", NULL, 1, 
                optionArgument(interfaceLoadBuilding, 0)
            ), 
            optionNew("scene", NULL, 1, 
                optionArgument(interfaceLoadScene, 0)
            )
        ),
        optionNew("building", NULL, 2, optionNew("reload", NULL, 0), optionNew("unwrap", NULL, 0))
    );

    return options;
}

void defineUi(struct Ui *ui){
    struct UiElement *blockHolder = uiElementInit(ui);
    // uiAddElement(blockHolder, ui->root, ABSOLUTE_PERCENTAGE, PERCENTAGE, 75, 5, 20, 90);
    uiAddElement(blockHolder, ui->root, ABSOLUTE_PERCENTAGE, PERCENTAGE, 16, 10, 68, 80);
    blockHolder->color = (struct ElementColor){40, 40, 40};

    // printf("blockHolder %p %p\n", &blockHolder, blockHolder.parent);
    // printf("%p\n", ui->uiElements->data[1].parent);
    
    struct UiElement *blockHolder2 = uiElementInit(ui);
    blockHolder2->color = (struct ElementColor){255, 0, 0};
    uiAddElement(blockHolder2, blockHolder, RELATIVE_PERCENTAGE, PERCENTAGE, 5, 5, 90, 90);
    // the command line
    // uiAddElement(ui, ui->root, ABSOLUTE, );

    uiBake(ui);
}

void showUi(struct BlockSupervisor *bs, struct Shader *shader, struct Ui *ui, GLint modelUniformLocation, GLint colorUniform){
    if(!interface->activeUi){
        return;
    }

    glDisable(GL_DEPTH_TEST);
    printf("ui dr\n");
    uiDraw(ui);
    printf("inactive ui\n");
    // glDisable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    useShader(shader);

    // SET_COLOR(colorUniform, RED);
    // glEnable(GL_SCISSOR_TEST);
    float height = textSetHeightPx(30);
    // printf("text start\n");
    float xOffset = 0.21f * 2.0f - 1.0f;
    float yOffset = 1.0f - 0.15f * 2.0f - height;
    SET_COLOR(colorUniform, WHITE);
    textDrawOnScreen(interface->text, interface->buffer, xOffset, yOffset, modelUniformLocation);
    // printf("text end\n");
    // commandLineDraw(interface->cmd, modelUniformLocation, colorUniform);
    // float endPos = textDrawOnScreen(interface->text, interface->buffer, 0, 0, modelUniformLocation);
    // char *end = "|";
    // textDrawOnScreen(interface->text, end, -1.0f + endPos, -1.0f + 0.02f, modelUniformLocation);

    float offset = yOffset - height;
    SET_COLOR(colorUniform, GRAY);

    if(interface->blockIsActive && bs->availableBlockTypesSize){
        for(BlockTypeId i = 0; i < bs->availableBlockTypesSize; i++){
            BlockTypeId id = bs->availableBlockTypes[i];
            struct BlockType *bt = &bs->blockTypes[id];
            if(strncmp(interface->buffer, bt->idStr, interface->bufferSize) == 0){
                textDrawOnScreen(interface->text, bt->idStr, xOffset, offset, modelUniformLocation);
                offset -= height;
            }
        }
    }
    else if(!interface->blockIsActive && bs->buildingTypes->size){
        for(BlockTypeId i = 0; i < bs->buildingTypes->size; i++){
            BlockTypeId id = bs->buildingTypes->data[i]->id;
            struct BuildingType *bt = bs->buildingTypes->data[id];
            if(strncmp(interface->buffer, bt->name, interface->bufferSize) == 0){
                textDrawOnScreen(interface->text, bt->name, xOffset, offset, modelUniformLocation);
                offset -= height;
            }
        }
    }
    // printf("draw text end\n");
    // glDisable(GL_SCISSOR_TEST);
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

    useShader(shader);
    struct BlockSupervisor *blockSupervisor = blockSupervisorInit(shader, modelUniformLocation);
    useShader(shader);
    text = textInit(shader, &g->screenRatio);
    useShader(shader);

    struct Option *options = optionsInit(blockSupervisor->blockTypesNames);
    optionPrint(options, 0);
    cmd = commandLineInit(options);

    interface = interfaceInit(cmd, blockSupervisor, g, text);

    // addBlock(blockSupervisor, PISTON, (BlockPosition){-5, 0, 0}, EAST);
    // addBlock(blockSupervisor, PISTON, (BlockPosition){-3, 0, 0}, SOUTH);
    // addBlock(blockSupervisor, PISTON, (BlockPosition){-1, 0, 0}, NORTH);
    // addBlock(blockSupervisor, PISTON, (BlockPosition){1, 0, 0}, WEST);
    // addBlock(blockSupervisor, PISTON, (BlockPosition){3, 0, 0}, UP);
    // addBlock(blockSupervisor, PISTON, (BlockPosition){5, 0, 0}, DOWN);

    // interfaceExportBuilding();
    // interfaceLoadBuilding();
    // interfaceAddBuilding();
    // buildingLoad(blockSupervisor, "/tmp/build");
    // buildingAdd(blockSupervisor, 0, (BlockPosition){0, 0, 0}, EAST);
    useShader(shader);

    // for(int z = 0; z < 10; z++){
    //     for(int y = 0; y < 10; y++){
    //         for(int x = 0; x < 10; x++){
    //             addBlock(blockSupervisor, PISTON, (BlockPosition){x - 5, y - 5, z - 5}, EAST);
    //         }
    //     }
    // }

    GLint maxTexSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    printf("max texture size %i\n", maxTexSize);

    // struct Vec3Vector *intersections = Vec3VectorInit();

    GLuint elementArrayBuffer;
    glGenBuffers(1, &elementArrayBuffer);

    mat4 cleanMat;
    glm_mat4_identity(cleanMat);

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

        interfaceProcess(interface, modelUniformLocation, colorUniform);
        // textDrawOnScreen(text, "UwU", -1, -1, modelUniformLocation);
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

        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)cleanMat);
        vec3 zero = {-0.5, -0.5, -0.5};
        drawPoint(zero, colorUniform);
        
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
        
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);

        SET_COLOR(colorUniform, GRAY);
        float trig[] = {
            -10, -0.5, -10, 0, 0,
            -10, -0.5, 10, 0, 0,
            10, -0.5, 10, 0, 0,
            10, -0.5, -10, 0, 0
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(trig), trig, GL_DYNAMIC_DRAW);
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

        // for(int z = 0; z < 3; z++){
        //     for(int y = 0; y < 3; y++){
        //         for(int x = 0; x < 3; x++){
        //             vec3 start = {2, 2, -1};
        //             vec3 end = {z - 1 + 2, y - 1 + 2, x - 1 - 1};
        //             drawLineWeight(start, end, shader->vbo, elementArrayBuffer, modelUniformLocation);
        //         }
        //     }
        // }
        // vec3 a = {-5.500000, -0.500000, -0.500000};
        // vec3 b = {5.500000, -0.500000, -0.500000}; 
        // drawLineWeight(a, b, shader->vbo, elementArrayBuffer, modelUniformLocation);

        glUniform4f(colorUniform, 0, 0, 0, 0);
        // drawChunks(blockSupervisor);
        useShader(shader);
        drawBlocks(blockSupervisor);
        buildingDraw(blockSupervisor);
        useShader(shader);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // glDepthMask(GL_FALSE);
        // glDepthFunc(GL_ALWAYS);
        // uiDraw(ui);
        
        interfaceDraw(modelUniformLocation, colorUniform, shader->vbo, elementArrayBuffer);
        
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)cleanMat);
        glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, (float*)cleanMat);
        glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, (float*)cleanMat);
        showUi(blockSupervisor, shader, ui, modelUniformLocation, colorUniform);

        processInput();
        glfwSwapBuffers(g->window);

        glfwPollEvents();
    }

    return 0;
}
