#include "graphics.h"

extern struct Graphics *g;

extern bool command;
extern char commandBuffer[MAX_COMMAND_BUFFER_SIZE];
extern size_t commandBufferSize;

extern bool mouseClick;
extern double xPos, yPos;

bool moved = false;

void processInput(){
    if(command){
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

void keyCallback(GLFWwindow *w, int key, int scancode, int action, int mods){
    UNUSED(w);
    UNUSED(scancode);
    if(action == GLFW_RELEASE){
        return;
    }

    if(key == GLFW_KEY_TAB){
        g->camIndex = (g->camIndex + 1) % g->camSize;
        g->camera = g->cams[g->camIndex];
    }

    if(key == '/' && !command){
        command = true;
        APPEND_STRING(commandBuffer, commandBufferSize, MAX_COMMAND_BUFFER_SIZE, '/');
    }
    else if(command && isprint(key)){
        char c = key;
        if(isalpha(key)){
            c = tolower(key);
            if(glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
                c = toupper(c);
            }
        }
        APPEND_STRING(commandBuffer, commandBufferSize, MAX_COMMAND_BUFFER_SIZE, c);
    }
    else if(key == GLFW_KEY_BACKSPACE && commandBufferSize){
        commandBuffer[--commandBufferSize] = 0;
        if(!commandBufferSize){
            command = false;
        }
    }
    else if(key == GLFW_KEY_ENTER){
        APPEND_STRING(commandBuffer, commandBufferSize, MAX_COMMAND_BUFFER_SIZE, '\0');
        command = false;
        printf("%s\n", commandBuffer);
    }
    else if(key == GLFW_KEY_ESCAPE){
        command = false;
    }
}

void cursorPosCallback(GLFWwindow *w, double x, double y){
    UNUSED(x);
    UNUSED(y);
    
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
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        moved = false;
        glfwGetCursorPos(g->window, &prevX, &prevY);
    }
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && !moved){
        glfwGetCursorPos(g->window, &xPos, &yPos);
        if(prevX == xPos && prevY == yPos){
            mouseClick = true;
        }
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
