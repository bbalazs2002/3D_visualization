#version 430 core

// VBO-ból érkezõ változók
layout (location = 0 ) in vec3 vs_in_pos;
layout (location = 1 ) in vec3 vs_in_norm;
layout (location = 2 ) in vec3 vs_in_merged;
layout (location = 3 ) in vec2 vs_in_tex;

// a pipeline-ban tovább adandó értékek
out vec3 vs_out_pos;

// uniforms
uniform mat4 world;
uniform mat4 viewProj;

void main()
{

	vec3 pos = vs_in_merged * .01 + vs_in_pos;
	vs_out_pos = (world * vec4( pos, 1 )).xyz;

	gl_Position = viewProj * world * vec4( pos, 1 );
}