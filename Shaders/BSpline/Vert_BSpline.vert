#version 430 core

#define BSPLINE_CTRL_POINTS_SSBO 1
#define BSPLINE_KNOTS_SSBO 2
#include "../Modules/ObjectTypes/BSpline/BSpline_uniforms.glsl"
#include "../Modules/ObjectTypes/BSpline/BSpline.glsl"

// camera
#include "../Modules/Camera/Camera_uniforms.glsl"
#include "../Modules/Camera/Camera.glsl"

void main()
{
    int div = max(2, bSplineData.division);
    float tStart = BSplineGetTStart(GetTParams(bSplineData.degree, bSplineData.knotCount));
    float tEnd = BSplineGetTEnd(GetTParams(bSplineData.degree, bSplineData.knotCount));
    float deltaT = (tEnd - tStart) / float(div - 1);
    int index = gl_VertexID;
    float t = tStart + deltaT * float(index);

    gl_Position = CameraViewProj(vec4(BSpline(BSplineParams(
        bSplineData.degree, t,
        bSplineData.knotCount, bSplineData.ctrlPointCount
    )), 1));
}