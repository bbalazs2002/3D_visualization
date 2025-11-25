#version 430

// pipeline-ból bejövõ per-fragment attribútumok
in vec3 vs_out_pos;
in vec3 vs_out_norm;
in vec2 vs_out_tex;

// kimenõ érték - a fragment színe
out vec4 fs_out_col;

// material
#include "../Modules/Material.glsl"
uniform Material material;

// click handler
layout(early_fragment_tests) in;
layout(std430, binding = 0) buffer ModelIDBuffer {
    vec4 currentModelID[];
};
void SetCurrentModelID(int i, vec4 val) {
    currentModelID[i] = val;
}
#define CLICK_HANDLER_SET_SSBO SetCurrentModelID
#include "../Modules/ClickHandler.glsl"

uniform vec3 cameraPos;

// light
#include "../Modules/Light.glsl"
uniform Light light;

void main()
{
	// click handler
    ClickHandler(ClickHandlerParams(windowSize, cursorPos, ivec2(gl_FragCoord.xy), modelID));

    /////////////////////////////////
    // DEBUG
    /////////////////////////////////
    // fs_out_col = vec4(vs_out_norm, 1);
    // fs_out_col = vec4(vs_out_tex, 0, 1);
    // fs_out_col = vec4(fract(vs_out_tex), 0.0, 1.0);
    // fs_out_col = vec4(1, .5, 0, 0);
    // return;

    // fragment normal
    vec3 norm = vs_out_norm;
    if (material.hasNormalTex > 0) {
        vec3 norm = texture(material.normalTex, vs_out_tex).rgb * 2.0 - 1.0;
    }
    norm = normalize(norm);

    // fs_out_col = vec4(norm * .5 + .5, 1);
    // return;

    if (material.hasEmissionTex > 0) {
        fs_out_col = vec4(texture(material.emissionTex, vs_out_tex).rgb, 1);
        return;
    }

    // light
    vec3 lightDir;
    float lightDist = 1.0;

    if (light.type == 0) {
        // Directional
        lightDir = normalize(-light.pos);
    } else {
        // Point
        vec3 lightVec = light.pos - vs_out_pos;
        lightDist = length(lightVec);
        lightDir = normalize(lightVec);
    }

    // Set directions
    vec3 viewDir = normalize(cameraPos - vs_out_pos);
    vec3 reflectDir = reflect(-lightDir, norm);

    // Ambient
    vec3 ambient = light.La * material.ambientColor;

    // Diffuse
    vec3 diffuseBase = vec3(1.);
    // vec3 diffuseBase = material.diffuseColor;
    if (material.hasDiffuseTex > 0) {
        diffuseBase *= texture(material.diffuseTex, vs_out_tex).rgb;
    }
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.Ld * diffuseBase * diff;

    // Specular
    vec3 specBase = material.specularColor;
    if (material.hasSpecularTex > 0)
        specBase *= texture(material.specularTex, vs_out_tex).rgb;

    float specStrength = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.Ls * specBase * specStrength;

    // Attenuáció (pontfény esetén)
    float attenuation = 1.0;
    if (light.type == 1) {
        attenuation = 1.0 / (light.constantAttenuation +
                             light.linearAttenuation * lightDist +
                             light.quadraticAttenuation * lightDist * lightDist);
    }

    vec3 result = (ambient + diffuse + specular) * attenuation;
    fs_out_col = vec4(result, 1.0);
}