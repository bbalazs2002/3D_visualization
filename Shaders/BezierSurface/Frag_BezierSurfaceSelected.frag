#version 430
layout(early_fragment_tests) in;

// kimenõ érték - a fragment színe
out vec4 fs_out_col;

uniform vec3 selColor = vec3(1, 0, 0);

void main()
{
	fs_out_col = vec4(selColor, 1);
}