#version 460

layout (location = 0) in vec3 fragColor;

layout (location = 0) out vec4 OutColor;

void main(){
    OutColor = vec4(fragColor, 1.0);
}