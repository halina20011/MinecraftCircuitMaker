#version 330 core
in vec2 FragTextureCords2;
uniform sampler2D textureSampler;
uniform vec4 color;

out vec4 FragmentColor;

void main(){
    // comment
    FragmentColor = texture(textureSampler, FragTextureCords2) * vec4(1 - color.a) + color * color.a;
}
