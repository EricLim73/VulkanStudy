#version 460

//  set where it will recieve its data (attribute description)
layout(location = 0) in vec2 position; 
layout(location = 1) in vec3 color; 

//  For Push Constant layout should match with the pushConstant implemented inside the program
layout(push_constant) uniform Push{
    mat2 transform;
    vec2 offset;    //  8byte   according to alignmentRules
    vec3 color;     //  16byte according to alignmentRules 
}push;  //  => Total 16 * 2 memory_size layout

//  layout(location = 0) out vec3 fragColor;    //  location is considered differently between "in" & "out"

void main(){
    gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
    //fragColor = color;
}