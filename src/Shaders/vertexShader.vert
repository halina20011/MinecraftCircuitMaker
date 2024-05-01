#version 330 core
in vec3 position;
in vec2 texture;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 FragTextureCords2;

void main(){
    gl_Position = projection * view * model * vec4(position, 1.0);
    FragTextureCords2 = texture;
}
