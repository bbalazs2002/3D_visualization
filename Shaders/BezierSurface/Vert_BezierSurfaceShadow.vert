#version 430 core

out vec3 vs_out_pos;

// BezierSurface
#define BEZIER_SURFACE_CTRL_POINTS_SSBO 1
#include "../Modules/ObjectTypes/BezierSurface/BezierSurface_uniforms.glsl"
#include "../Modules/ObjectTypes/BezierSurface/BezierSurface.glsl"

#define LIGHT_LIGHTS_SSBO 3
#define LIGHT_SPACE_MATRICES_SSBO 4
#include "../Modules/Light/Light_uniforms.glsl"

uniform int lightID;
uniform int faceID;

// Triangles
void main()
{
    int divu = int(bezierSurfaceData.division.x);
    float deltau = 1.f / float(divu - 1);
    int divv = int(bezierSurfaceData.division.y);
    float deltav = 1.f / float(divv - 1);

    int tid = int(floor(gl_VertexID / 3));  // id of the current triangle

    int vr = (bezierSurfaceData.division.x - 1) * 2 * 3;        // verteces in a row
    int row = int(floor(gl_VertexID / vr));                     // id of the current row
    int correctID = gl_VertexID - vr * row;                     // collapse to one row
    int col = int(floor(correctID / 6));                        // id of the current column
    correctID = correctID - col * 6;                            // collapse to one column

    float u = col
            + (tid % 2) * (1 - int(floor(correctID / 5)))       // +w if the correctID is 3 or 4
            + (1 - (tid % 2)) * int(floor(correctID / 2));      // +w if the correctID is 2
    float v = row
            + (gl_VertexID % 2);                                // +h if the ID is odd

    vec4 p = vec4(BezierSurface(
        BezierSurfaceParams(u * deltau, v * deltav, bezierSurfaceData.ctrlPointCount)
    ), 1);
    gl_Position = lightSpaceMatrices[int(lightSources[lightID].direction_lightSpace.w) + faceID] * p;
}