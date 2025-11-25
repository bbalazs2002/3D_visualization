#version 430 core

// SSBO bound to binding point 1
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];   // flexible array member
};
vec4 BezierGetCtrlPoint(int i) {
    return ctrlPoints[i];
}
#define BEZIER_GET_CTRL_POINT BezierGetCtrlPoint

#include "../Modules/BezierUtils.glsl"

uniform mat4 viewProj;
uniform int ctrlPointCount = 3;
uniform int division = 500;

void main()
{
    int index = gl_VertexID;
    int div = division;
    float deltaT = 1.f / float(div - 1);

	gl_Position = viewProj * vec4(Bezier(
        BezierParams(index * deltaT, ctrlPointCount)
    ), 1);
}