#version 330 core
in vec2 position;
uniform mat4 transform;
void main(){
    vec4 pos  = transform * vec4(position.x, position.y, 0.0, 1.0);
    gl_Position = vec4(pos.x - 1, pos.y - 1, pos.z, pos.a);
}
