#version 330 core
in vec2 uiPosition;
in vec4 uiColor;
out vec4 uiColorOut;
void main(){
    uiColorOut = uiColor;
    gl_Position = vec4(uiPosition, 0.0, 1.0);
}
