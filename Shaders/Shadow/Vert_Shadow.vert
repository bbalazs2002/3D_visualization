#version 430

// Variables coming from the VBO
layout( location = 0 ) in vec3 vs_in_pos;

// External parameters of the shader
uniform mat4 world;
uniform mat4 viewProj;

void main()
{
	gl_Position = viewProj * world * vec4( vs_in_pos, 1 );
}