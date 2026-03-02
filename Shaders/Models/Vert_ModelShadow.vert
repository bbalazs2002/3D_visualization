#version 430 core
layout (location = 0 ) in vec3 vs_in_pos;

#include "../Modules/Transform/Transform_uniforms.glsl"
#include "../Modules/Transform/Transform.glsl"

#define LIGHT_LIGHTS_SSBO 3
#define LIGHT_SPACE_MATRICES_SSBO 4
#include "../Modules/Light/Light_uniforms.glsl"

uniform int lightID;

void main()
{
    mat4 viewProj = lightSpaceMatrices[int(lightSources[lightID].direction_lightSpace.w)];
    gl_Position = viewProj * Transform(vec4(vs_in_pos, 1.0));
}