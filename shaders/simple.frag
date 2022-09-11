#version 460

layout (location = 0) in vec3 fragColor;

layout (location = 0) out vec4 OutColor;

layout(push_constant) uniform Push{
    mat4 transform; //  MVP matrix
    mat4 normalMatrix;    
}push;  

void main(){
    OutColor = vec4(fragColor, 1.0);
}