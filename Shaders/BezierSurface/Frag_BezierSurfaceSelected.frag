#version 430

// kimenõ érték - a fragment színe
out vec4 fs_out_col;

// color
#include "../Modules/Color/Color_uniforms.glsl"
#include "../Modules/Color/Color.glsl"

void main()
{
	fs_out_col = Color();
}