float CalculateShadow(vec4 fragPosLightSpace, uint layer, float bias) {
    // 1. Perspective divide (NDC koordináták: -1 és 1 között)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // 2. Skálázás [0, 1] tartományba (mivel a textúra koordináták itt kezdõdnek)
    projCoords = projCoords * 0.5f + 0.5f;

    // 3. Ha a pont távolabb van, mint a farPlane, ne legyen árnyékban
    if (projCoords.z > 1.0f) return 1.0f;     // not in shadow

    // 4. Mintavételezés a tömbbõl
    float closestDepth = texture(lightShadowMapArray, vec3(projCoords.xy, float(layer))).r; 
    
    // 5. Aktuális pixel mélysége
    float currentDepth = projCoords.z;

    // 6. Összehasonlítás (egyszerû shadow bias-szal az acne ellen)
    float shadow = currentDepth - bias > closestDepth ? 0.0f : 1.0f;      // 0: in shadow; 1 not in shadow

    return shadow;
}

struct LightCalculateContributionParams {
    Light light;
    vec3 position;
    vec3 norm;          // normalised
    vec3 viewDir;       // normalised
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
};
vec3 LightCalculateContribution(LightCalculateContributionParams params) {
    vec3 lightDir;
    float attenuation = 1.0;
    float spotIntensity = 1.0;
    int flags = int(params.light.flags_angle_shadow.x);
    
    // 1. Determine Light Direction and Attenuation
    if ((LIGHT_FLAG_IS_DIR & flags) != 0u) {
        lightDir = normalize(-params.light.direction.xyz);
    }
    else { // Point or Spot Light
        vec3 lightToFrag = params.light.position.xyz - params.position;
        float dist = length(lightToFrag);
        lightDir = normalize(lightToFrag);

        // Attenuation calculation (constant, linear, quadratic)
        attenuation = 1.0 / (params.light.La_const.w + params.light.Ld_linear.w * dist + params.light.Ls_quadratic.w * dist * dist);
        
        if ((LIGHT_FLAG_IS_SPOT & flags) != 0u) {
            // Spot Light Calculation
            vec3 spotDir = normalize(vec3(0,-1,0)); // normalize(params.light.direction.xyz);
            float theta = dot(lightDir, -spotDir);      // cosine of angle between light ray and spot direction

            float innerCutOff = cos(params.light.flags_angle_shadow.y);
            float outerCutOff = cos(params.light.flags_angle_shadow.z);

            if (theta > outerCutOff) {
                // Smooth fade from inner to outer cutoff (soft edges)
                spotIntensity = smoothstep(outerCutOff, innerCutOff, theta);
            } else {
                // Fragment is outside the spot cone
                spotIntensity = 0.0;
            }
        }
    }

    // Calculat shadow
    float shadow = 1.0f;
    if(dot(params.norm, lightDir) <= 0) {
        shadow = 0.f;
    }
    else if ((LIGHT_FLAG_CASTS_SHADOW & flags) != 0u) {
        mat4 viewProj = params.light.lightSpaceMatrix;
        uint layer = uint(params.light.flags_angle_shadow.w);
        float bias = max(0.05 * (1.0 - dot(params.norm, lightDir)), 0.005);
        shadow = CalculateShadow(viewProj * vec4(params.position, 1), layer, bias);
    }
    
    // If the light is dimmed out by spot or attenuation, skip the expensive calculations
    if (attenuation <= 0.0 || spotIntensity <= 0.0) {
        return vec3(0.0);
    }
    
    // 2. Diffuse Component
    float diff = max(dot(params.norm, lightDir), 0.0);
    vec3 diffuse = params.light.Ld_linear.xyz * params.diffuseColor * diff;
    
    // 3. Specular Component (Blinn-Phong)
    vec3 halfDir = normalize(lightDir + params.viewDir);
    float spec = pow(max(dot(params.norm, halfDir), 0.0), params.shininess);
    vec3 specular = params.light.Ls_quadratic.xyz * params.specularColor * spec;

    // 4. Combine and apply attenuation/spot factor
    return shadow * (diffuse + specular) * attenuation * spotIntensity;
}

struct LightCalculateParams{
    vec3 norm;          // normal vector at the fragment
    vec3 viewDir;       // fragment to camera direction
    vec3 position;      // fragment position in world space
    float[13] material; // material properties packed as follows:
                        //  [0-2]:  ambient color
                        //  [3-5]:  diffuse color
                        //  [6-8]:  specular color
                        //  [9-11]: emission color
                        //  [12]:   shininess
};
vec3 LightCalculate(LightCalculateParams params) {
    // normalise vectors
    vec3 norm = normalize(params.norm);
    vec3 viewDir = normalize(params.viewDir);

    // unpack material properties
    vec3 ambientColor  = vec3(params.material[0],  params.material[1], params.material[2]);
    vec3 diffuseColor  = vec3(params.material[3],  params.material[4], params.material[5]);
    vec3 specularColor = vec3(params.material[6],  params.material[7], params.material[8]);
    vec3 emissionColor = vec3(params.material[9],  params.material[10], params.material[11]);
    float shininess    = params.material[12];

    // --- 1. Calculate Ambient Light (The base color component) ---
    // Start with the base ambient contribution, derived from the material's ambient color
    vec3 totalLight = ambientColor;

    // --- 2. Iterate and Accumulate Light Contributions ---
    for (int i = 0; i < lightData.lightCount; i++) {
        // Ambient light from the light source itself (La_const.xyz)
        totalLight += lightSources[i].La_const.xyz * ambientColor; 
        
        // Diffuse and Specular components
        totalLight += LightCalculateContribution(LightCalculateContributionParams(
            lightSources[i],
            params.position,
            norm, viewDir,
            diffuseColor, specularColor,
            shininess
        ));
    }

    return max(vec3(0), min(vec3(1.f), totalLight + emissionColor));
}