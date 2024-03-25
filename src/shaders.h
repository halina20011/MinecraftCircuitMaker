
#define FRAGMENT_SHADER "#version 330 core\n"\
    "uniform vec3 color;\n"\
    "out vec4 outColor;\n"\
    "void main(){\n"\
    "    outColor = vec4(color, 1.0);\n"\
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
