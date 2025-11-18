#version 430

// Variables coming from the VBO
layout( location = 0 ) in vec3 vs_in_pos;
layout( location = 1 ) in vec3 vs_in_norm;
layout( location = 2 ) in vec2 vs_in_tex;

// Values to be passed on the pipeline
out vec3 vs_out_pos;
out vec3 vs_out_norm;
out vec2 vs_out_tex;

// External parameters of the shader
uniform mat4 world;
uniform mat4 worldIT;
uniform mat4 viewProj;

void main()
{
	gl_Position = viewProj * world * vec4( vs_in_pos, 1 );
	vs_out_pos  = (world   * vec4(vs_in_pos,  1)).xyz;
	vs_out_norm = (worldIT * vec4(vs_in_norm, 0)).xyz;
	vs_out_tex = vs_in_tex;
}