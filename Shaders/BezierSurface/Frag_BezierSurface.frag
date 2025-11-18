#version 430
layout(early_fragment_tests) in;
layout(std430, binding = 0) buffer ModelIDBuffer {
    vec4 currentModelID[];
};

// pipeline-ból bejövõ per-fragment attribútumok
in vec3 vs_out_pos;
in vec3 vs_out_norm;
in vec2 vs_out_tex;

// kimenõ érték - a fragment színe
out vec4 fs_out_col;

// material
struct Material {
    vec3 diffuseColor;
    float shininess;
    vec3 specularColor;
    int hasDiffuseTex;
    vec3 ambientColor;
    int hasSpecularTex;
    int hasEmissionTex;
    int hasNormalTex;
    sampler2D diffuseTex;
    sampler2D specularTex;
    sampler2D emissionTex;
    sampler2D normalTex;
};
uniform Material material;

// cursor position in window coords (same as gl_FragCoord.xy)
uniform vec2 windowSize = vec2(0, 0);
uniform vec2 cursorPos = vec2(0, 0);
uniform int modelID = -1;

uniform vec3 cameraPos;

// light
struct Light {
	vec3 pos;
	float constantAttenuation;
	vec3 La;
	float linearAttenuation;
	vec3 Ld;
	float quadraticAttenuation;
	vec3 Ls;
	int type;
};
uniform Light light;

void main()
{
	ivec2 frag = ivec2(gl_FragCoord.xy);
    ivec2 cursor = ivec2(cursorPos.x, windowSize.y - cursorPos.y);
	// index is 0 if frag == cursor, 1 if frag != cursor
	// save model below cursor to index 0
	currentModelID[1 - int(frag == cursor)] = vec4(modelID);

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