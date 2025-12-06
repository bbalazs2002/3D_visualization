#version 430 core

// VBO-ból érkezõ változók
layout (location = 0 ) in vec3 vs_in_pos;
layout (location = 1 ) in vec3 vs_in_norm;
layout (location = 3 ) in vec2 vs_in_tex;

// Transform
#include "../Modules/Transform/Transform_uniforms.glsl"
#include "../Modules/Transform/Transform.glsl"

// Camera
#include "../Modules/Camera/Camera_uniforms.glsl"
#include "../Modules/Camera/Camera.glsl"

void main()
{
	gl_Position = CameraViewProj(Transform(vec4( vs_in_pos, 1 )));
}