#version 430 core

// SSBO bound to binding point 1
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];   // flexible array member
};

uniform mat4 viewProj;

void main() {
    int index = gl_VertexID;
    gl_Position = viewProj * ctrlPoints[index];
}