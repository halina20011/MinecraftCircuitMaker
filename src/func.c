// #include "func.h"

#include "graphics.h"

#include "commandLine.h"

#include "interface.h"

extern struct Graphics *g;
extern struct CommandLine *cmd;
extern struct Interface *interface;

bool moved = false;
// extern bool placeBlock;

void processInput(){
    if(interface->activeBuffer){
        return;
    }

    float cameraSpeed = 5.f * g->deltaTime;
    if(glfwGetKey(g->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        cameraSpeed *= 2.0;
    }
    
    if(glfwGetKey(g->window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        cameraSpeed *= 0.05;
    }

    vec3 offset;
    glm_vec3_scale(g->camera->cameraFront, cameraSpeed, offset);

    if(glfwGetKey(g->window, GLFW_KEY_W) == GLFW_PRESS){
        glm_vec3_add(g->camera->cameraPos, offset, g->camera->cameraPos);
    }
    if(glfwGetKey(g->window, GLFW_KEY_S) == GLFW_PRESS){
        glm_vec3_sub(g->camera->cameraPos, offset, g->camera->cameraPos);
    }

    glm_vec3_scale(g->camera->cameraUp, cameraSpeed, offset);

    if(glfwGetKey(g->window, GLFW_KEY_Q) == GLFW_PRESS){
        glm_vec3_add(g->camera->cameraPos, offset, g->camera->cameraPos);
    }
    if(glfwGetKey(g->window, GLFW_KEY_E) == GLFW_PRESS){
        glm_vec3_sub(g->camera->cameraPos, offset, g->camera->cameraPos);
    }

    glm_vec3_cross(g->camera->cameraFront, g->camera->cameraUp, offset);
    glm_vec3_normalize(offset);
    glm_vec3_scale(offset, cameraSpeed, offset);
    if(glfwGetKey(g->window, GLFW_KEY_A) == GLFW_PRESS){
        glm_vec3_sub(g->camera->cameraPos, offset, g->camera->cameraPos);
    }
    if(glfwGetKey(g->window, GLFW_KEY_D) == GLFW_PRESS){
        glm_vec3_add(g->camera->cameraPos, offset, g->camera->cameraPos);
    }
}

#define UPDATE_CURRENT(index, size){\
    if(key == GLFW_KEY_DOWN){\
        if(index == 0){\
            index = size - 1;\
        }\
        else{\
            interface->currBlockIndex--;\
        }\
    }\
    else if(key == GLFW_KEY_UP){\
        index = (index + 1) % size;\
    }\
}

void keyCallback(GLFWwindow *w, int key, int scancode, int action, int mods){
    UNUSED(w);
    UNUSED(scancode);
    UNUSED(mods);
    if(action == GLFW_RELEASE){
        return;
    }

    if(key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT){
        interface->blockIsActive = !interface->blockIsActive;
    }

    if(!interface->activeUi){
        // switch camera
        if(key == GLFW_KEY_TAB){
            g->camIndex = (g->camIndex + 1) % g->camSize;
            g->camera = g->cams[g->camIndex];
        }

        // switch block type
        if(interface->blockIsActive){
            UPDATE_CURRENT(interface->currBlockIndex, interface->bs->availableBlockTypesSize);
        }
        else if(interface->bs->buildingTypes->size){
            UPDATE_CURRENT(interface->currBuildingIndex, interface->bs->buildingTypes->size);
        }
    }

    if(!interface->activeBuffer){
        return;
    }

    // start buffer line
    if(key == GLFW_KEY_BACKSPACE && interface->bufferSize){
        interface->buffer[--interface->bufferSize] = 0;
        if(!interface->bufferSize){
            interface->activeBuffer = false;
            interface->activeCmd = false;
            interface->activeUi = false;
        }
    }
    else if(glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        switch(key){
            case 'C':
                // printf("copy text\n");
                commandLineCopy(w);
                break;
            case 'V':
                commandLinePaste();
                break;
            case 'W':
                // printf("remove word\n");
                for(; interface->bufferSize != 0; interface->bufferSize--){
                    if(interface->buffer[interface->bufferSize] == ' '){
                        break;
                    } 
                }
                interface->buffer[interface->bufferSize] = '\0';
                break;
        }
    }
    else if(key == GLFW_KEY_ENTER){
        // printf("%s\n", cmd->buffer);
        if(!interface->activeUi){
            commandLineExecute(cmd);
        }
        else{
            interfaceSelectBlock();
        }
        interface->activeBuffer = false;
        interface->activeCmd = false;
        interface->activeUi = false;
        interface->bufferSize = 0;
        interface->buffer[0] = 0;
    }
    else if(key == GLFW_KEY_ESCAPE){
        interface->buffer[0] = 0;
        interface->bufferSize = 0;
        interface->activeBuffer = false;
        interface->activeCmd = false;
        interface->activeUi = false;
    }
}

void characterCallback(GLFWwindow *window, uint32_t codepoint){
    if((uint32_t)(UINT8_MAX) < codepoint){
        return;
    }
    uint8_t c = codepoint;
    // printf("%c", c);
    
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        return;
    }

    if(!interface->activeBuffer){
        if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
            interface->facing++;
            if(interface->facing == 4){
                interface->facing = 0;
            }
        }

        if('1' <= codepoint && codepoint <= '6'){
            interface->rotate = c - '0';
        }
    }

    if(!interface->activeCmd && c == ' '){
        interface->activeUi = !interface->activeUi;
        interface->activeBuffer = interface->activeUi;
        interface->bufferSize = 0;
        interface->buffer[0] = 0;
        return;
    }

    // start buffer line
    if(!interface->activeCmd && !interface->activeUi && c == '/'){
        interfaceBuffer('/');
        interface->activeBuffer = true;
        interface->activeCmd = true;
    }
    else if(interface->activeBuffer && isprint(c)){
        interfaceBuffer(c);
        if(!interface->activeUi){
            struct Option *option = commandLineCurrOption(cmd);
            optionPrint(option, 0);
        }
    }
}

void cursorPosCallback(GLFWwindow *w, double x, double y){
    UNUSED(x);
    UNUSED(y);
    
    interface->screenX = x;
    interface->screenY = y;

    moved = true;
    // only allow to move view when the left button is pressed
    if(!glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT)){
        g->camera->prevX = -1;
        return;
    }
    if(g->camera->prevX == -1){
        g->camera->prevX = x;
        g->camera->prevY = y;
    }

    float xOffset = x - g->camera->prevX;
    float yOffset = g->camera->prevY - y;

    g->camera->prevX = x;
    g->camera->prevY = y;

    float sensitivity = 0.5f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    g->camera->yaw += xOffset;
    g->camera->pitch += yOffset;

    if(89 < g->camera->pitch){
        g->camera->pitch = 89;
    }
    if(g->camera->pitch < -89){
        g->camera->pitch = -89.0f;
    }

    float yawRad = glm_rad(g->camera->yaw);
    float pitchRad = glm_rad(g->camera->pitch);

    g->camera->cameraFront[0] = cos(yawRad) * cos(pitchRad);
    g->camera->cameraFront[1] = sin(pitchRad);
    g->camera->cameraFront[2] = sin(yawRad) * cos(pitchRad);

    glm_vec3_normalize(g->camera->cameraFront);
}

void scrollCallback(GLFWwindow *w, double x, double y){
    UNUSED(w);
    UNUSED(x);
    g->camera->fov -= (float)y;
    if(g->camera->fov < 1.0){
        g->camera->fov = 1;
    }
    else if(90 < g->camera->fov){
        g->camera->fov = 90;
    }
}

void framebufferSizeCallback(GLFWwindow *w, int width, int height){
    UNUSED(w);
    g->width = width;
    g->height = height;
    g->screenRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}

double prevX, prevY;
void mouseButtonCallback(GLFWwindow *w, int button, int action, int mods){
    UNUSED(w);
    UNUSED(mods);
    if(action == GLFW_PRESS){
        moved = false;
    }

    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && !moved){
        interface->mouseClick = true;
    }

    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE && !moved){
        interface->rightClick = true;
    }
}

uint8_t *readFile(const char fileName[], size_t *rSize){
    FILE *file = fopen(fileName, "rb");
    if(!file){
        fprintf(stderr, "unable to open file: %s\n", fileName);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    void *data = malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    *rSize = size;

    return data;
}
