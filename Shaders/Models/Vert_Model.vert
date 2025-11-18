#version 430 core

// VBO-ból érkezõ változók
layout (location = 0 ) in vec3 vs_in_pos;
layout (location = 1 ) in vec3 vs_in_norm;
layout (location = 2 ) in vec2 vs_in_tex;

// a pipeline-ban tovább adandó értékek
out vec3 vs_out_pos;
out vec2 vs_out_tex;
out vec3 vs_out_norm;

// uniforms
uniform mat4 world;
uniform mat4 viewProj;

void main()
{
	vs_out_norm = normalize(vs_in_norm);

	// vs_out_tex = vs_in_tex;
	vs_out_tex = vec2(vs_in_tex.x, 1.f - vs_in_tex.y);
	// vs_out_tex = vec2(1.f - vs_in_tex.x, vs_in_tex.y);
	// vs_out_tex = vec2(1.f - vs_in_tex.x, 1.f - vs_in_tex.y);

	// vs_out_tex = vs_in_tex.yx;
	// vs_out_tex = vec2(vs_in_tex.y, 1.f - vs_in_tex.x);
	// vs_out_tex = vec2(1.f - vs_in_tex.y, vs_in_tex.x);
	// vs_out_tex = vec2(1.f - vs_in_tex.y, 1.f - vs_in_tex.x);

	vs_out_pos = (world * vec4( vs_in_pos, 1 )).xyz;

	gl_Position = viewProj * world * vec4( vs_in_pos, 1 );
}