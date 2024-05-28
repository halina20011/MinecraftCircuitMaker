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
    if(cmd->active){
        return;
    }

    // if(glfwGetKey(g->window, GLFW_KEY_ENTER) == GLFW_PRESS){
    //     placeBlock = true;
    // }

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

void keyCallback(GLFWwindow *w, int key, int scancode, int action, int mods){
    UNUSED(w);
    UNUSED(scancode);
    UNUSED(mods);
    if(action == GLFW_RELEASE){
        return;
    }

    // switch camera
    if(key == GLFW_KEY_TAB){
        g->camIndex = (g->camIndex + 1) % g->camSize;
        g->camera = g->cams[g->camIndex];
    }

    // switch block type
    if(key == GLFW_KEY_DOWN){
        if(interface->currBlockIndex == 0){
            interface->currBlockIndex = interface->bs->availableBlockTypesSize - 1;
        }
        else{
            interface->currBlockIndex--;
        }
    }
    else if(key == GLFW_KEY_UP){
        interface->currBlockIndex = (interface->currBlockIndex + 1) % interface->bs->availableBlockTypesSize;
    }

    if(!cmd->active){
        return;
    }

    // start command line
    if(key == GLFW_KEY_BACKSPACE && cmd->commandSize){
        cmd->command[--cmd->commandSize] = 0;
        if(!cmd->commandSize){
            cmd->active = false;
        }
    }
    else if(glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        switch(key){
            case 'C':
                // printf("copy text\n");
                commandLineCopy(cmd, w);
                break;
            case 'V':
                commandLinePaste(cmd);
                break;
            case 'W':
                // printf("remove word\n");
                for(; cmd->commandSize != 0; cmd->commandSize--){
                    if(cmd->command[cmd->commandSize] == ' '){
                        break;
                    } 
                }
                cmd->command[cmd->commandSize] = '\0';
                break;
        }
    }
    else if(key == GLFW_KEY_ENTER){
        cmd->active = false;
        // printf("%s\n", cmd->command);
        commandLineExecute(cmd);
        cmd->commandSize = 0;
    }
    else if(key == GLFW_KEY_ESCAPE){
        cmd->commandSize = 0;
        cmd->active = false;
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

    // start command line
    if(c == '/' && !cmd->active){
        cmd->active = true;
        commandLineAdd(cmd, '/');
    }
    else if(cmd->active && isprint(c)){
        // if(isalpha(key)){
        //     c = tolower(key);
            // if(c == 'c' && glfwGetKey)(
        //     if(glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        //         c = toupper(c);
        //     }
        // }
        commandLineAdd(cmd, c);
        struct Option *option = commandLineCurrOption(cmd);
        // printf("option %p\n", option);
        optionPrint(option, 0);
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

    // if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
    //     moved = false;
    //     prevX = interface->screenX;
    //     prevY = interface->screenY;
    //     // glfwGetCursorPos(g->window, &prevX, &prevY);
    // }
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && !moved){
        // glfwGetCursorPos(g->window, &interface->screenX, &interface->screenY);
        // if(interface->screenX == prevX && interface->screenY == prevY){
            interface->mouseClick = true;
        // }
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
