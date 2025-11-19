#version 430 core

// SSBO bound to binding point 1
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];   // flexible array member
};

uniform mat4 viewProj;

void main()
{
    int index = gl_VertexID;
    // int div = division;
    // float deltaT = 1.f / float(div - 1);
	// gl_Position = viewProj * vec4(DiscreteCurve(index * deltaT), 1);
    gl_Position = viewProj * ctrlPoints[index];
}