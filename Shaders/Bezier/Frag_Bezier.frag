#version 430
layout(early_fragment_tests) in;

// kimenõ érték - a fragment színe
out vec4 fs_out_col;

uniform vec3 color = vec3(1, 0, 1);
uniform vec3 selColor = vec3(1, 0, 0);
uniform int isSelection = 0;

void main()
{
	int mult = min(isSelection, 1);
	fs_out_col = vec4(mult * selColor + (1 - mult) * color, 1);
}