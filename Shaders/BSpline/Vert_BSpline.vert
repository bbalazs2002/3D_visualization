#version 430 core

// === Control points SSBO ===
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];
};
vec4 BSplineGetCtrlPoint(int i) {
    return ctrlPoints[i];
}
#define BSPLINE_GET_CTRL_POINT BSplineGetCtrlPoint

// === Knot vector SSBO ===
layout(std430, binding = 2) buffer knotSSBO {
    float knots[];
};
float BSplineGetKnot(int i) {
    return knots[i];
}
#define BSPLINE_GET_KNOT BSplineGetKnot

uniform mat4 viewProj;
uniform int ctrlPointCount = 4; // Kontrollpontok száma
uniform int knotCount = 8;      // Csomópontok száma
uniform int degree = 3;         // Fokszám
uniform int division = 50;      // Kiértékelési lépések

#include "../Modules/BSplineUtils.glsl"

void main()
{
    int div = max(2, division); 
    float tStart = GetTStart(GetTParams(degree, knotCount));
    float tEnd = GetTEnd(GetTParams(degree, knotCount));
    float deltaT = (tEnd - tStart) / float(div - 1);
    int index = gl_VertexID;
    float t = tStart + deltaT * float(index);

    gl_Position = viewProj * vec4(BSpline(BSplineParams(
        degree, t,
        knotCount, ctrlPointCount
    )), 1);

    // gl_Position = viewProj * ctrlPoints[index];
}