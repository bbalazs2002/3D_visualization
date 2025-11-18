#version 430 core

// SSBO bound to binding point 1
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];   // flexible array member
};

layout(points) in;
layout(points, max_vertices = 64) out;

uniform mat4 viewProj;
uniform int ctrlPointCount = 3;

void main() {

    for (int i = 0; i < ctrlPointCount; ++i) {
        gl_Position = viewProj * ctrlPoints[i];
        EmitVertex();
    }
    EndPrimitive();
}