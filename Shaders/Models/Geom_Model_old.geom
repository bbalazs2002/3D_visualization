#version 430 core

#define GEOMETRY_SHADER

in vec2 vs_out_tex[];
in vec3 vs_out_norm[];

out vec2 gs_out_tex;
out vec3 gs_out_norm;
out vec3 gs_out_pos;

layout(triangles) in;  
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 world;
uniform mat4 viewProj;

//
// Include common file with helper functions
//
#include "Glsl_common.glsl"

void main() {

    //
    // TRIALNGLES
    //

    // Bottom
    gs_out_tex = vs_out_tex[0];
    gs_out_norm = vs_out_norm[0];
    gs_out_pos = gl_in[0].gl_Position.xyz;
    gl_Position = viewProj * gl_in[0].gl_Position;
    EmitVertex();

    gs_out_tex = vs_out_tex[1];
    gs_out_norm = vs_out_norm[1];
    gs_out_pos = gl_in[1].gl_Position.xyz;
    gl_Position = viewProj * gl_in[1].gl_Position;
    EmitVertex();
    gs_out_tex = vs_out_tex[2];
    gs_out_norm = vs_out_norm[2];
    gs_out_pos = gl_in[2].gl_Position.xyz;
    gl_Position = viewProj * gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();

}