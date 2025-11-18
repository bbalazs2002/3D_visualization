#version 430

// pipeline-ból bejövõ per-fragment attribútumok
in vec3 vs_out_pos;
in vec3 vs_out_norm;
in vec2 vs_out_tex;

out vec4 fs_out_col;

// Textures
uniform sampler2D coneMap;

void main()
{
	fs_out_col = vec4( vs_out_tex,0.0,1.0);
	// fs_out_col = texture(coneMap, vs_out_tex);
}