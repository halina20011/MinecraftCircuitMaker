
#define FRAGMENT_SHADER "#version 330 core\n"\
    "uniform vec3 color;\n"\
    "out vec4 outColor;\n"\
    "void main(){\n"\
    "    outColor = vec4(color, 1.0);\n"\
    "}\n"

#define VERTEX_SHADER "#version 330 core\n"\
    "in vec2 position;\n"\
    "uniform mat4 transform;\n"\
    "void main(){\n"\
    "    vec4 pos  = transform * vec4(position.x, position.y, 0.0, 1.0);\n"\
    "    gl_Position = vec4(pos.x - 1, pos.y - 1, pos.z, pos.a);\n"\
    "}\n"
