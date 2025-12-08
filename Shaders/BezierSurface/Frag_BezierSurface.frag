#version 430

// pipeline-ból bejövõ per-fragment attribútumok
in vec3 vs_out_pos;
in vec3 vs_out_norm;
in vec2 vs_out_tex;

// kimenõ érték - a fragment színe
out vec4 fs_out_col;

// click handler
#define CLICK_HANDLER_MODEL_SSBO 0
#include "../Modules/ClickHandler/ClickHandler_uniforms.glsl"
#include "../Modules/ClickHandler/ClickHandler.glsl"

// material
#include "../Modules/Material/Material_uniforms.glsl"
#include "../Modules/Material/Material.glsl"

// light
#define LIGHT_LIGHTS_SSBO 2
#include "../Modules/Light/Light_uniforms.glsl"
#include "../Modules/Light/Light.glsl"

// camera
#include "../Modules/Camera/Camera_uniforms.glsl"
#include "../Modules/Camera/Camera.glsl"

void main()
{
    // click handler
    ClickHandler();

    // material
    float[13] colors = MaterialPrepare(vs_out_tex);

    // narmalise vectors
    vec3 norm = normalize(vs_out_norm);
    vec3 viewDir = normalize(cameraData.eye - vs_out_pos);

    // calculate lights
    fs_out_col = vec4(LightCalculate(LightCalculateParams(
        norm, viewDir, vs_out_pos, colors
    )), 1.0);
}