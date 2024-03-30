
#define FRAGMENT_SHADER "#version 330 core\n"\
    "uniform vec3 color;\n"\
    "out vec4 outColor;\n"\
    "void main(){\n"\
    "    outColor = vec4(color, 1.0);\n"\
    "}\n"

#define FRAGMENT_UI_SHADER "#version 330 core\n"\
    "in vec4 uiColorOut;\n"\
    "out vec4 aaaColor;\n"\
    "void main(){\n"\
    "    aaaColor = uiColorOut;\n"\
    "}\n"

#define VERTEX_UI_SHADER "#version 330 core\n"\
    "in vec2 uiPosition;\n"\
    "in vec4 uiColor;\n"\
    "out vec4 uiColorOut;\n"\
    "void main(){\n"\
    "    uiColorOut = uiColor;\n"\
    "    gl_Position = vec4(uiPosition, 0.0, 1.0);\n"\
    "}\n"

#define VERTEX_SHADER "#version 330 core\n"\
    "in vec3 position;\n"\
    "uniform mat4 model;\n"\
    "uniform mat4 view;\n"\
    "uniform mat4 projection;\n"\
    "\n"\
    "void main(){\n"\
    "    gl_Position = projection * view * model * vec4(position, 1.0);\n"\
    "}\n"
