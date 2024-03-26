#include "graphics.h"
#include <cglm/mat4.h>
#include <cglm/vec4.h>

float screenRatio;
float globAngle = 0;

float deltaTime = 0, lastFrame = 0;

vec3 cameraPos = {0, 0, 3};

vec3 cameraFront = {0, 0, -1};
vec3 cameraUp = {0, 1, 0};

float yaw = -90, pitch = 0;
float fov = 45;

float posX = 0, posY = 0, posZ = 0;
float rotX = 0, rotZ = 0;

int windowWidth, windowHeight;

#define IGNORE __attribute__ ((unused))
#define UNUSED(x) (void)(x)
#define UNUSEDS(...) (void)(__VA_ARGS__)

void GLAPIENTRY messageCallback(IGNORE GLenum source, IGNORE GLenum type, IGNORE GLuint id, GLenum severity, IGNORE GLsizei length, const GLchar* message, IGNORE const void* userParam){
    // UNUSEDS(source, id, length, userParam);
    const char *messageString = (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" );
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", messageString, type, severity, message );
}

GLint compileShader(const char *shaderSource, int type){
    // printf("%s\n", shaderSource);
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char * const*)&shaderSource, NULL);
    glCompileShader(shader);

    char buffer[512];
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(shader, 512, NULL, buffer);
    if(status != GL_TRUE){
        fprintf(stderr, "%s\n", buffer);
        exit(1);
    }

    return shader;
}

void framebufferSizeCallback(GLFWwindow *w, int width, int height){
    UNUSED(w);
    // windowWidth = width;
    // windowHeight = height;
    screenRatio = (float)width / (float)height;
    printf("%i %i %f\n", windowWidth, windowHeight, screenRatio);
    glViewport(0, 0, windowWidth, windowHeight);
}

void processInput(GLFWwindow *w){
    float cameraSpeed = 5.f * deltaTime;
    vec3 offset;
    glm_vec3_scale(cameraFront, cameraSpeed, offset);

    if(glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS){
        glm_vec3_add(cameraPos, offset, cameraPos);
    }
    if(glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS){
        glm_vec3_sub(cameraPos, offset, cameraPos);
    }

    glm_vec3_scale(cameraUp, cameraSpeed, offset);

    if(glfwGetKey(w, GLFW_KEY_Q) == GLFW_PRESS){
        glm_vec3_add(cameraPos, offset, cameraPos);
    }
    if(glfwGetKey(w, GLFW_KEY_E) == GLFW_PRESS){
        glm_vec3_sub(cameraPos, offset, cameraPos);
    }

    glm_vec3_cross(cameraFront, cameraUp, offset);
    glm_vec3_normalize(offset);
    glm_vec3_scale(offset, cameraSpeed, offset);
    if(glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS){
        glm_vec3_sub(cameraPos, offset, cameraPos);
    }
    if(glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS){
        glm_vec3_add(cameraPos, offset, cameraPos);
    }
}

float prevX = -1, prevY = 0;
void cursorPosCallback(GLFWwindow *w, double x, double y){
    if(!glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT)){
        prevX = -1;
        return;
    }
    UNUSED(w);
    UNUSED(x);
    UNUSED(y);
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
    fov -= (float)y;
    if(fov < 1.0){
        fov = 1;
    }
    else if(90 < fov){
        fov = 90;
    }
}

void drawLine(int x1, int y1, int z1, int x2, int y2, int z2){
    float line[] = {
        x1, y1, z1, 
        x2, y2, z2, 
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, 2);
}

// wrapper around glGetUniformLocation to show errors
GLint getUniformLocation(GLuint program, const GLchar *name){
    GLint _uniformLocation = glGetUniformLocation(program, name);
    if(_uniformLocation == -1){
        fprintf(stderr, "failed to get uniform location from \"%s\"\n", name);
        exit(-1);
    }

    return _uniformLocation;
}

void graphicsInit(){
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

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    screenRatio = (float)windowWidth / (float)windowHeight;

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        fprintf(stderr, "failed to initialize GLEW\n");
        exit(1);
    }

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, 0);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const char vertexShaderSource[] = VERTEX_SHADER;
    const char fragmentShaderSource[] = FRAGMENT_SHADER;

    GLint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glLinkProgram(program);
    glUseProgram(program);

    glClearColor(0, 0, 0,1);

    GLint posAttrib = glGetAttribLocation(program, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);

    GLint modelUniformLocation      = getUniformLocation(program, "model");
    GLint viewUniformLocation       = getUniformLocation(program, "view");
    GLint projectionUniformLocation = getUniformLocation(program, "projection");
    
    GLuint colorUniform = glGetUniformLocation(program, "color");
    glUniform3f(colorUniform, 0.5, 0.5, 0.5);

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
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

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

    // vec3 cameraTarget = {0, 0, 0};
    // vec3 cameraDirection = {0, 0, 0};
    // glm_vec3_sub(cameraPos, cameraTarget, cameraDirection);

    // right axis
    // vec3 up = {0, 1, 0};

    // vec3 cameraRight;
    // glm_vec3_cross(up, cameraDirection, cameraRight);
    // glm_vec3_normalize(cameraRight);

    // glm_vec3_cross(cameraDirection, cameraRight, cameraUp);
    // glm_vec3_print(cameraUp, stdout);

    int prev = 0, curr;
    int couter = 0;
    while(!glfwWindowShouldClose(window)){
        float currFrame = glfwGetTime();
        deltaTime = currFrame - lastFrame;
        lastFrame = currFrame;

        curr = currFrame;
        if(prev != curr){
            prev = curr;
            printf("coutner %i\n", couter);
            couter = 1;
        }
        else{
            couter++;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 view, projection;
        glm_mat4_identity(view);
        glm_mat4_identity(projection);

        // glm_ortho(0, 800.0f, 0, 600.0f, -10.0f, 200.0f, projection);
        glm_perspective(glm_rad(fov), screenRatio, 0.1f, 100.0f, projection);

        vec3 center;
        glm_vec3_add(cameraPos, cameraFront, center);
        glm_lookat(cameraPos, center, cameraUp, view);
        
        glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, (float*)projection);

        for(size_t i = 0; i < cubePositionsSize; i++){
            mat4 model;
            glm_mat4_identity(model);
            glm_translate(model, cubePositions[i]);

            float angle = glm_rad(20 * i);
            glm_rotate(model, angle, (vec3){1, 0.3, 0.5});

            glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);

            glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

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
        // arrowInit(1, 0);
        // // y axis
        // glUniform3f(colorUniform, 0, 1, 0);
        // // z axis
        // glUniform3f(colorUniform, 0, 0, 1);
        glfwSwapBuffers(window);
        glfwPollEvents();
        processInput(window);
    }

    glDeleteProgram(program);
    glfwTerminate();
}
