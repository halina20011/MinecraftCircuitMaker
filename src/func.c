#include "graphics.h"

extern struct Graphics *g;

extern bool command;
extern char commandBuffer[MAX_COMMAND_BUFFER_SIZE];
extern size_t commandBufferSize;

extern vec3 cameraPos;

extern vec3 cameraFront;
extern vec3 cameraUp;

extern float yaw, pitch, fov;

void processInput(){
    if(command){
        return;
    }

    float cameraSpeed = 5.f * g->deltaTime;
    if(glfwGetKey(g->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        cameraSpeed *= 2.0;
    }
    vec3 offset;
    glm_vec3_scale(cameraFront, cameraSpeed, offset);

    if(glfwGetKey(g->window, GLFW_KEY_W) == GLFW_PRESS){
        glm_vec3_add(cameraPos, offset, cameraPos);
    }
    if(glfwGetKey(g->window, GLFW_KEY_S) == GLFW_PRESS){
        glm_vec3_sub(cameraPos, offset, cameraPos);
    }

    glm_vec3_scale(cameraUp, cameraSpeed, offset);

    if(glfwGetKey(g->window, GLFW_KEY_Q) == GLFW_PRESS){
        glm_vec3_add(cameraPos, offset, cameraPos);
    }
    if(glfwGetKey(g->window, GLFW_KEY_E) == GLFW_PRESS){
        glm_vec3_sub(cameraPos, offset, cameraPos);
    }

    glm_vec3_cross(cameraFront, cameraUp, offset);
    glm_vec3_normalize(offset);
    glm_vec3_scale(offset, cameraSpeed, offset);
    if(glfwGetKey(g->window, GLFW_KEY_A) == GLFW_PRESS){
        glm_vec3_sub(cameraPos, offset, cameraPos);
    }
    if(glfwGetKey(g->window, GLFW_KEY_D) == GLFW_PRESS){
        glm_vec3_add(cameraPos, offset, cameraPos);
    }
}

void keyCallback(GLFWwindow *w, int key, int scancode, int action, int mods){
    UNUSED(w);
    UNUSED(scancode);
    if(action == GLFW_RELEASE){
        return;
    }

    if(key == '/' && !command){
        command = true;
        commandBufferSize = 0;
    }
    else if(command && isprint(key)){
        printf("%c", key);
        fflush(stdout);
        APPEND_STRING(commandBuffer, commandBufferSize, MAX_COMMAND_BUFFER_SIZE, key);
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

float prevX = -1, prevY = 0;
void cursorPosCallback(GLFWwindow *w, double x, double y){
    UNUSED(x);
    UNUSED(y);
    if(!glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT)){
        prevX = -1;
        return;
    }
    if(prevX == -1){
        prevX = x;
        prevY = y;
    }

    float xOffset = x - prevX;
    float yOffset = prevY - y;

    prevX = x;
    prevY = y;
    float sensitivity = 0.5f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if(89 < pitch){
        pitch = 89;
    }
    if(pitch < -89){
        pitch = -89.0f;
    }

    float yawRad = glm_rad(yaw);
    float pitchRad = glm_rad(pitch);

    cameraFront[0] = cos(yawRad) * cos(pitchRad);
    cameraFront[1] = sin(pitchRad);
    cameraFront[2] = sin(yawRad) * cos(pitchRad);

    glm_vec3_normalize(cameraFront);
}

void scrollCallback(GLFWwindow *w, double x, double y){
    UNUSED(w);
    UNUSED(x);
    fov -= (float)y;
    if(fov < 1.0){
        fov = 1;
    }
    else if(90 < fov){
        fov = 90;
    }
}

void framebufferSizeCallback(GLFWwindow *w, int width, int height){
    UNUSED(w);
    g->width = width;
    g->height = height;
    g->screenRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}
