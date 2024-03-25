#include "graphics.h"
#include <cglm/mat4.h>
#include <cglm/vec4.h>

float screenRatio;
float globAngle = 0;

float posX = 0, posY = 0, posZ = 0;
float rotX = 0, rotZ = 0;

int windowWidth, windowHeight;

#define IGNORE __attribute__ ((unused))
#define UNUSED(x) (void)(x)
#define UNUSEDS(...) (void)(__VA_ARGS__)

#define MOVE(var, val, keyIncrease, keyDecrease){\
    case keyIncrease:\
        var += val;\
        break;\
    case keyDecrease:\
        var -= val;\
        break;\
}

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

void keyCallback(GLFWwindow *w, int key, int scancode, int action, int mods){
    UNUSED(w);
    UNUSED(scancode);
    UNUSED(action);
    UNUSED(mods);
    switch(key){
        MOVE(posX, 0.2, GLFW_KEY_D, GLFW_KEY_A);
        MOVE(posY, 0.2, GLFW_KEY_W, GLFW_KEY_S);
        MOVE(posZ, 0.2, GLFW_KEY_E, GLFW_KEY_Q);

        MOVE(rotX, 1, GLFW_KEY_DOWN, GLFW_KEY_UP);
        MOVE(rotZ, 1, GLFW_KEY_RIGHT, GLFW_KEY_LEFT);
        default:
            break;
    }
}

void cursorPosCallback(GLFWwindow *w, double x, double y){
    UNUSED(w);
    UNUSED(x);
    UNUSED(y);
    // double r = x + y;
    // printf("%f %f\n", x, y);
}

void scrollCallback(GLFWwindow *w, double x, double y){
    UNUSED(w);
    printf("%f %f\n", x, y);
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
    // glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetKeyCallback(window, keyCallback);
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

    glClearColor(1,1,1,1);

    GLint posAttrib = glGetAttribLocation(program, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);

    GLint modelUniformLocation      = getUniformLocation(program, "model");
    GLint viewUniformLocation       = getUniformLocation(program, "view");
    GLint projectionUniformLocation = getUniformLocation(program, "projection");
    
    GLuint colorUniform = glGetUniformLocation(program, "color");
    // glUniform3f(colorUniform, 1, 1, 1);
    glUniform3f(colorUniform, 0.5, 0.5, 0.5);

    // float x = 0, y = 0, _width = 1, _height = 1;
    // float z = 0;
    // float rectangle[] = {
    //     x           , y + _height, z,// A,
    //     x + _width   , y + _height, z, // b
    //     x           , y, z, // d
    //     x + _width   , y, z // c
    // };

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
    // glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_STATIC_DRAW);

    // mat4 test;
    // glm_mat4_identity(test);
    //
    // glm_scale(test, (vec3){1, 0.5, 1});
    // glm_mat4_print(test, stdout);
    
    int prev = 0, curr;
    int couter = 0;
    while(!glfwWindowShouldClose(window)){
        curr = (int)glfwGetTime();

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
        glm_perspective(glm_rad(45.0f), screenRatio, 0.1f, 100.0f, projection);

        float rad = 10;
        float camX = sin(glfwGetTime()) * rad;
        float camZ = cos(glfwGetTime()) * rad;
        glm_lookat((vec3){camX, 0, camZ}, (vec3){0, 0, 0}, (vec3){0, 1, 0}, view);
        
        // glm_mat4_print(projection, stdout);
        glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, (float*)projection);

        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, (vec3){posX, posY, posZ});
        // glm_scale(model, (vec3){0.5, 1, 1});
        glm_rotate(model, glm_rad(rotX), (vec3){1, 0, 0});
        glm_rotate(model, glm_rad(rotZ), (vec3){0, 0, 1});

        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);

        glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
        glUniform3f(colorUniform, 0.5, 0.5, 0.5);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glUniform3f(colorUniform, 0, 0, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(program);
    glfwTerminate();
}
