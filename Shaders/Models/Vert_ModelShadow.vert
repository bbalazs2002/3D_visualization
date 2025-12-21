#version 330 core
layout (location = 0 ) in vec3 vs_in_pos;

#include "../Modules/Camera/Camera_uniforms.glsl"
#include "../Modules/Camera/Camera.glsl"

#include "../Modules/Transform/Transform_uniforms.glsl"
#include "../Modules/Transform/Transform.glsl"

void main()
{
    gl_Position = CameraViewProj(Transform(vec4(vs_in_pos, 1.0)));
}