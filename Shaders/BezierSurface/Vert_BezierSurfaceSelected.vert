#version 430 core

// BezierSurface
#define BEZIER_SURFACE_CTRL_POINTS_SSBO 1
#include "../Modules/ObjectTypes/BezierSurface/BezierSurface_uniforms.glsl"

// camera
#include "../Modules/Camera/Camera_uniforms.glsl"
#include "../Modules/Camera/Camera.glsl"

void main()
{
	int index = gl_VertexID;
	gl_Position = CameraViewProj(bezierSurfaceCtrlPoints[index]);
}