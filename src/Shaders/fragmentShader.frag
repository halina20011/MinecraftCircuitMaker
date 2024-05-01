#version 330 core
in vec2 FragTextureCords2;
uniform sampler2D textureSampler;

out vec4 FragmentColor;

void main(){
    FragmentColor = texture(textureSampler, FragTextureCords2);
}
