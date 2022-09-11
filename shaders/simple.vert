#version 460
 
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 color; 
layout(location = 2) in vec3 normal; 
layout(location = 3) in vec2 uv; 


layout(location = 0) out vec3 fragColor;
 
layout(push_constant) uniform Push{
    mat4 transform; //  MVP matrix
    mat4 modelMatrix;    
}push;   

//  inputs for light calculation should always be normalized
const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));   // set directional lighting at (1,-3,-1)  
//  NOTE:   In Directional light, every ray is considered parallel, other lights requires 
//          additional calculation for per vertex light direction(ex. point-vertexPoint) 

void main(){
    gl_Position = push.transform * vec4(position, 1.0); 
    //gl_Position.y = -gl_Position.y; 
    
    //  bc 4th R&C represent movement, we cast modelTransform into 3X3 and mult it with ObjectSpace "normal"
    //  We get normal's viewed inside WorldSpace where our light source is currently at.
    //  TODO:   This method is just quick dirty trick so needs change in future
    vec3 normalWorldSpace = normalize(mat3(push.modelMatrix)*normal);
    //  dot can result "-" if normal faces opposite to lightSource == dark so needs to be "0"
    float lightIntensity = max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0);


    fragColor = lightIntensity * color;
}