#version 430 core

// DiscreteCurve
#define DISCRETE_CURVE_CTRL_POINTS_SSBO 1
#include "../Modules/ObjectTypes/DiscreteCurve/DiscreteCurve_uniforms.glsl"

// camera
#include "../Modules/Camera/Camera_uniforms.glsl"
#include "../Modules/Camera/Camera.glsl"

void main()
{
    int index = gl_VertexID;
    gl_Position = CameraViewProj(DiscreteCurveCtrlPoints[index]);
}