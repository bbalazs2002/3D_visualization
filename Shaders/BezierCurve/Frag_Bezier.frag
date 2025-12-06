#version 430
layout(early_fragment_tests) in;

// kimenõ érték - a fragment színe
out vec4 fs_out_col;

// Color
#include "../Modules/Color/Color_uniforms.glsl"
#include "../Modules/Color/Color.glsl"

void main()
{
	fs_out_col = Color();
}