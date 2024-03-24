#include "graphics.h"
#include <cglm/mat4.h>
#include <cglm/vec4.h>

float globAngle = 0;
float angle = 0;

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
    glViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow *w, int key, int scancode, int action, int mods){
    switch(key){
        case GLFW_KEY_LEFT:
            globAngle += 1.0;
            break;
        case GLFW_KEY_RIGHT:
            globAngle -= 1.0;
            break;
        case GLFW_KEY_UP:
            angle += 1.0;
            break;
        case GLFW_KEY_DOWN:
            angle -= 1.0;
            break;
        default:
            break;
    }
}

void cursorPosCallback(GLFWwindow *w, double x, double y){
    printf("%f %f\n", x, y);
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        fprintf(stderr, "failed to initialize GLEW\n");
        exit(1);
    }

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
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);

    GLint tranformLoc = glGetUniformLocation(program, "transform");
    
    GLuint colorUniform = glGetUniformLocation(program, "color");
    glUniform3f(colorUniform, 0.5, 0.5, 0.5);

    float x = 0, y = 0, width = 0.5, height = 0.5;
    float rectangle[] = {
        x           , y + height, // A
        x + width   , y + height, // b
        x           , y, // d
        x + width   , y  // c
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_DYNAMIC_DRAW);

    vec3 position = {1.5, 1, 0};
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);

        mat4 trans;
        glm_mat4_identity(trans);

        // 1) rotate the object in the local space
        // 2) move the object to the global space
        // 3) rotate global space around the pivot (0, 0, 0)

        glm_rotate_at(trans, (vec3){0, 0, 0}, glm_rad(globAngle), (vec3){0, 0, 1});
        glm_translate(trans, position);
        glm_rotate_z(trans, glm_rad(angle), trans);

        glUniformMatrix4fv(tranformLoc, 1, GL_FALSE, (float*)trans);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glfwSwapBuffers(window);
    }

    glDeleteProgram(program);
    glfwTerminate();
}
