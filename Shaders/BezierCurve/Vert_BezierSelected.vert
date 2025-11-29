#version 430 core

// SSBO bound to binding point 1
#define BEZIER_CURVE_CTRL_POINTS_SSBO 1
#include "../Modules/ObjectTypes/BezierCurve/BezierCurve_uniforms.glsl"

// camera
#include "../Modules/Camera/Camera_uniforms.glsl"
#include "../Modules/Camera/Camera.glsl"

void main() {
    int index = gl_VertexID;
    gl_Position = CameraViewProj(bezierCurveCtrlPoints[index]);
}