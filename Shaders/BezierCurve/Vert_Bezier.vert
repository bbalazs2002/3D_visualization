#version 430 core

// BezierCurve
#define BEZIER_CURVE_CTRL_POINTS_SSBO 1
#include "../Modules/ObjectTypes/BezierCurve/BezierCurve_uniforms.glsl"
#include "../Modules/ObjectTypes/BezierCurve/BezierCurve.glsl"

// camera
#include "../Modules/Camera/Camera_uniforms.glsl"
#include "../Modules/Camera/Camera.glsl"

void main()
{
    int index = gl_VertexID;
    float deltaT = 1.f / float(bezierCurveData.division - 1);
    float t = float(index) * deltaT;

	gl_Position = CameraViewProj(vec4(Bezier(
        BezierParams(t, bezierCurveData.ctrlPointCount)
    ), 1));
}