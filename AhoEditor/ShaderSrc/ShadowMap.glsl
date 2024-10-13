#type vertex
#version 460 core
layout(location = 0) in vec3 aPos;

uniform mat4 u_LightSpaceMatrix;
uniform mat4 u_Model;

void main() {
    gl_Position = u_LightSpaceMatrix * model * vec4(aPos, 1.0);
}


#type fragment
#version 460 core
void main() {
    // do nothing
}