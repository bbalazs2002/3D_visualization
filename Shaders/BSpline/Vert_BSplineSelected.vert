#version 430 core

// BSpline
#define BSPLINE_CTRL_POINTS_SSBO 1
#define BSPLINE_KNOTS_SSBO 2
#include "../Modules/ObjectTypes/BSpline/BSpline_uniforms.glsl"

// camera
#include "../Modules/Camera/Camera_uniforms.glsl"
#include "../Modules/Camera/Camera.glsl"

void main() {
    int index = gl_VertexID;
    gl_Position = CameraViewProj(BSplineCtrlPoints[index]);
}