#version 430 core

// SSBO bound to binding point 1
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];   // flexible array member
};
vec4 BezierGetCtrlPoint(int i) {
    return ctrlPoints[i];
}
#define BEZIER_GET_CTRL_POINT BezierGetCtrlPoint

#include "../Modules/BezierSurfaceUtils.glsl"

// out vec3 vs_out_col;
out vec3 vs_out_pos;
out vec3 vs_out_norm;
out vec2 vs_out_tex;

uniform mat4 viewProj;
uniform ivec2 ctrlPointCount = ivec2(5, 5);
uniform ivec2 division = ivec2(10,10);

// Point cloud
/*
void main() {

    int index = gl_VertexID;

    int u = index % division.x;
    int v = int(floor(float(index) / float(division.x)));

    int divu = int(division.x);
    float deltau = 1.f / float(divu - 1);

    int divv = int(division.y);
    float deltav = 1.f / float(divv - 1);
    
    vec4 p = vec4(BSurface(u * deltau, v * deltav), 1);
    gl_Position = viewProj * p;
}
*/

// Triangles
void main()
{
    int divu = int(division.x);
    float deltau = 1.f / float(divu - 1);
    int divv = int(division.y);
    float deltav = 1.f / float(divv - 1);

    int tid = int(floor(gl_VertexID / 3));  // id of the current triangle

    int vr = (division.x - 1) * 2 * 3;      // verteces in a row
    int row = int(floor(gl_VertexID / vr)); // id of the current row
    int correctID = gl_VertexID - vr * row; // collapse to one row
    int col = int(floor(correctID / 6));    // id of the current column
    correctID = correctID - col * 6;        // collapse to one column

    float u = col * deltau
            + (tid % 2) * (1 - int(floor(correctID / 5))) * deltau           // +w if the correctID is 3 or 4
            + (1 - (tid % 2)) * int(floor(correctID / 2)) * deltau;          // +w if the correctID is 2
    float v = row * deltav
            + (gl_VertexID % 2) * deltav;                                    // +h if the ID is odd

    vs_out_tex = vec2(u,v);

    vec4 p = vec4(BezierSurface(
        BezierSurfaceParams(u, v, ctrlPointCount)
    ), 1);
    gl_Position = viewProj * p;
    vs_out_pos = (viewProj * p).xyz;

    vec3 T_u = BezierSurface_du(
        BezierSurfaceParams(u, v, ctrlPointCount)
    );
    vec3 T_v = BezierSurface_dv(
        BezierSurfaceParams(u, v, ctrlPointCount)
    );
    vs_out_norm = normalize(cross(T_v, T_u));
}

// Triangle strip
/*
void main()
{
    // TRIANGLE_STRIP
    float w = 1.f / division.x;
    float h = 1.f / division.y;

    int period = 2 + cols * 4;
    int blockID = int(floor(gl_VertexID / period));
    int correctID = gl_VertexID - blockID * period;

    int sgn = int(round(max(0.0, sign(period / 2.0 - correctID))));       // (-inf; 0] => 0, (0; +inf) => 1

    float x = position.x
        + sgn * ((correctID - (correctID % 2)) / 2.0) * w                                     // position if the vertex is in the upper row of the block
        + (1 - sgn) * (cols - ((correctID + (correctID % 2)) / 2.0 - (cols + 1.0))) * w;      // position if the vertex is in the lower row of the block

    float z = position.z
        + h * (
            blockID * 2.0                                   // number of rows before this block
            + sgn * (correctID % 2)                         // relative position to the block for the upper row
            + (1 - sgn) * (1.0 + (1.0 - (correctID % 2)))   // relative position to the block for the lower row
        );
  
    vec2 texPos = vec2((x - position.x) / width, (z - position.z) / height);
    vs_out_color = texture(texImage, texPos);
    // vs_out_color = vec4(.0, 1., .0, 1.);
    // vec4 hTex = texture(hightMapImage, texPos);
    gl_Position = viewProj * world * vec4(x, position.y + texture(hightMapImage, texPos).g * heightMult, z, 1.0);
    // gl_Position = viewProj * world * vec4(x, 0.0, z, 1.0);
}
*/