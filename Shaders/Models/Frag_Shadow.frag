#version 450 core

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform int numLights;
uniform Light lights[16];               // up to 16 lights
uniform mat4 lightVP[16];               // light view-projection matrices
uniform sampler2DArray shadowMaps;      // all shadow maps stored in a texture array

in vec3 FragPos;   // world-space position of the fragment
in vec3 Normal;    // world-space normal
out vec4 FragColor;

// ---- shadow calculation ----
float ShadowCalculation(int lightIndex, vec3 fragPosWorld)
{
    // Transform fragment to light space
    vec4 lightSpacePos = lightVP[lightIndex] * vec4(fragPosWorld, 1.0);

    // Perspective divide → normalized device coords
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    // Transform to [0,1] range for texture lookup
    projCoords = projCoords * 0.5 + 0.5;

    // Early out: if fragment is outside the light’s frustum
    if (projCoords.z > 1.0) return 0.0;

    // Depth from shadow map
    float closestDepth = texture(shadowMaps, vec3(projCoords.xy, lightIndex)).r;

    // Depth of current fragment from light’s POV
    float currentDepth = projCoords.z;

    // Bias to avoid shadow acne
    float bias = 0.001;

    // Shadow test
    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewColor = vec3(0.0);

    for (int i = 0; i < numLights; i++) {
        vec3 lightDir = normalize(lights[i].position - FragPos);

        // Basic diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = lights[i].color * diff * lights[i].intensity;

        // Shadow factor
        float shadow = ShadowCalculation(i, FragPos);

        // Apply shadow
        viewColor += (1.0 - shadow) * diffuse;
    }

    FragColor = vec4(viewColor, 1.0);
}
