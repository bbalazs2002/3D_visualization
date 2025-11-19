#version 430 core

// === Control points SSBO ===
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];
};

uniform mat4 viewProj;

void main() {
    int index = gl_VertexID;
    gl_Position = viewProj * ctrlPoints[index];
}