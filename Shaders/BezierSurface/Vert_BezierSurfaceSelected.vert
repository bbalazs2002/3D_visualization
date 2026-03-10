#version 430 core

// BezierSurface
#define BEZIER_SURFACE_CTRL_POINTS_SSBO 1
#include "../Modules/ObjectTypes/BezierSurface/BezierSurface_uniforms.glsl"

// camera
#include "../Modules/Camera/Camera_uniforms.glsl"
#include "../Modules/Camera/Camera.glsl"

void main()
{
    // POINT CLOUD
	int index = gl_VertexID;
	gl_Position = CameraViewProj(bezierSurfaceCtrlPoints[index]);

/* TRIANGLES
	int divu = int(bezierSurfaceData.division.x);
    int divv = int(bezierSurfaceData.division.y);

    int tid = int(floor(gl_VertexID / 3));  // id of the current triangle

    int vr = (bezierSurfaceData.division.x - 1) * 2 * 3;        // verteces in a row
    int row = int(floor(gl_VertexID / vr));                     // id of the current row
    int correctID = gl_VertexID - vr * row;                     // collapse to one row
    int col = int(floor(correctID / 6));                        // id of the current column
    correctID = correctID - col * 6;                            // collapse to one column

    int u = col
            + (tid % 2) * (1 - int(floor(correctID / 5)))           // +w if the correctID is 3 or 4
            + (1 - (tid % 2)) * int(floor(correctID / 2));          // +w if the correctID is 2
    int v = row
            + (gl_VertexID % 2);                                    // +h if the ID is odd

    vec4 p = bezierSurfaceCtrlPoints[u * bezierSurfaceData.ctrlPointCount.y + v];

    gl_Position = CameraViewProj(vec4(p.x, p.y, p.z, 1));
    */
}