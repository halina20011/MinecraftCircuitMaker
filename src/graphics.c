#include "graphics.h"

#define UNPACK3(val) val[0], val[1], val[2]

void GLAPIENTRY messageCallback(IGNORE GLenum source, IGNORE GLenum type, IGNORE GLuint id, GLenum severity, IGNORE GLsizei length, const GLchar* message, IGNORE const void* userParam){
    // UNUSEDS(source, id, length, userParam);
    const char *messageString = (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" );
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", messageString, type, severity, message );
}

void drawLine(float x1, float y1, float z1, float x2, float y2, float z2){
    float line[] = {
        x1, y1, z1, 0, 0,
        x2, y2, z2, 0, 0
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, 2);
}

// void drawLineWeight(float x1, float y1, float z1, float x2, float y2, float z2){
//     vec3 start = {x1, y1, z1};
//     vec3 end = {x2, y2, z2};
//
//     vec3 startDir = {};
//     glm_vec3_sub(start, end, startDir);
// }

void drawLineVec(vec3 start, vec3 end){
    float line[] = {
        start[0], start[1], start[2], 0, 0,
        end[0], end[1], end[2], 0, 0
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, 2);
}

void drawDirection(vec3 start, vec3 end, float size){
    vec3 target = {};
    glm_vec3_sub(end, start, target);
    glm_vec3_normalize(target);
    glm_vec3_scale(target, size, target);
    glm_vec3_add(start, target, target);
    drawLineVec(start, target);
}

void drawArrow(vec3 end, float scale){
    vec3 arrow;
    glm_vec3_normalize(end);
    glm_vec3_scale(end, scale, arrow);
    // const float angle = 45;
    // float distance = sin(glm_rad(angle)) * 0.1;
    // UNUSED(distance);
    float line[] = {
        0, 0, 0, 0, 0,
        arrow[0], arrow[1], arrow[2], 0, 0
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, 2);
}

void drawPoint(vec3 pos, GLint colorUniform){
    float cx = pos[0];
    float cy = pos[1];
    float cz = pos[2];
    const float s = 0.2;
    
    SET_COLOR(colorUniform, RED);
    drawLine(cx - s , cy, cz, cx + s, cy, cz);
    SET_COLOR(colorUniform, BLUE);
    drawLine(cx, cy - s , cz, cx, cy + s, cz);
    SET_COLOR(colorUniform, GREEN);
    drawLine(cx, cy, cz - s, cx, cy, cz + s);
}

void drawLineDirection(vec3 pos, vec3 relDirection){
    // glm_vec3_normalize(relDirection);
    // glm_vec3_scale(relDirection, 1000, relDirection);
    // glm_vec3_add(pos, relDirection, relDirection);

    drawLine(pos[0], pos[1], pos[2], relDirection[0], relDirection[1], relDirection[2]);
}

struct Graphics *graphicsInit(){
    struct Graphics *g = malloc(sizeof(struct Graphics));
    g->deltaTime = 0;
    g->lastFrame = 0;

    if(glfwInit() != GL_TRUE){
        fprintf(stderr, "failed to initialize GLFW\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Circuit Maker", NULL, NULL);
    if(!window){
        fprintf(stderr, "failed to open glfw window\n");
        exit(1);
    }
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, g);

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, characterCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwGetFramebufferSize(window, &g->width, &g->height);
    g->screenRatio = (float)g->width / (float)g->height;

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        fprintf(stderr, "failed to initialize GLEW\n");
        exit(1);
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, 0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_SCISSOR_TEST);

    glClearColor(0, 0, 0, 1);
    
    // const size_t cubePositionsSize = 0;

    // vec3 cameraTarget = {0, 0, 0};
    // vec3 cameraDirection;
    // glm_vec3_sub(cameraPos, cameraTarget, cameraDirection);

    // glDeleteProgram(program);
    
    g->window = window;

    return g;
    // glfwTerminate();
}

void graphicsAddCameras(struct Graphics *g, struct Camera **cams, size_t size){
    g->camIndex = 0;
    g->cams = cams;
    g->camSize = size;
    g->camera = g->cams[0];
}

GLuint loadTexture(uint8_t *data, int width, int height){
    GLuint textureName;
    glGenTextures(1, &textureName);
    printf("textureName: %i %i %i\n", textureName, width, height);
    glBindTexture(GL_TEXTURE_2D, textureName);

    // printf("char %c => texure name: %i avg %f\n", i, textureName, sum / (float)(w * h));
    // text->asciiMap[i].bearingLeft = offsetLeft;
    // printf("t => %c %i %i\n", i, i, textureName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // for(int i = 0; i < width * height * 4; i++){
    //     data[i] = 255;
    // }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    
    return textureName;
}
