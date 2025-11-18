#version 430

layout(std430, binding = 0) buffer Positions {
	vec4 pos[];
};

// Input: a single point
layout(points) in;

// Output: three verteces in a V shape
layout(line_strip, max_vertices = 3) out;

// Input attributes from the vertex shader
in uint vs_out_vertexID[];
in vec3 vs_out_fragPos[];   // texture-space position for each vertex
in vec3 color[];
// in vec3 color[];

out vec3 vs_out_color;

// External parameters of the shader
uniform mat4 viewProj;
uniform int SSBOPadding = 5;
uniform sampler2D coneMap;

void main() {

    vs_out_color = color[0];
    // vs_out_color = vec3(1,0,0);
    uint index = vs_out_vertexID[0] + SSBOPadding + 2;
	mat4 invM = mat4(pos[1], pos[2], pos[3], pos[4]);	// texture to world transformation

    vec3 a = vec3(pos[index].xy, texture(coneMap, pos[index].xy).x);
    vec3 v = (pos[index + 1] - vec4(a, 1)).xyz;
    vec3 a1 = a + 2. * v;
    vec3 a2 = a + 2. * v * vec3(-1, -1, 1);

    gl_Position = viewProj * invM * vec4(a1, 1);
    EmitVertex();
    gl_Position = viewProj * invM * vec4(a, 1);
    EmitVertex();
    gl_Position = viewProj * invM * vec4(a2, 1);
    EmitVertex();

    EndPrimitive();
}