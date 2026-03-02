/**
 * Helper function to determine the CubeMap face index based on a direction vector.
 * The indexing follows the order used in the mat_buffer:
 * 0: +X, 1: -X, 2: +Y, 3: -Y, 4: +Z, 5: -Z
 */
int GetCubeFaceIndex(vec3 direction) {
    vec3 absDir = abs(direction);
    float maxVal = max(absDir.x, max(absDir.y, absDir.z));
    
    if (maxVal == absDir.x) {
        return (direction.x > 0.0) ? 0 : 1;
    } else if (maxVal == absDir.y) {
        return (direction.y > 0.0) ? 2 : 3;
    } else {
        return (direction.z > 0.0) ? 4 : 5;
    }
}

float Calculate2DShadow(vec4 fragPosLightSpace, uint layer, float bias) {
    // 1. Perspective divide (NDC coordinates: range [-1, 1])
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // 2. Transform to [0, 1] range (to match texture coordinate space)
    projCoords = projCoords * 0.5f + 0.5f;

    // 3. If the point is further than the far plane, it is not in shadow
    if (projCoords.z > 1.0f) return 1.0f;

    // 4. Sample the depth from the shadow map array
    float closestDepth = texture(light2DShadowMapArray, vec3(projCoords.xy, float(layer))).r; 
    
    // 5. Get current pixel depth
    float currentDepth = projCoords.z;

    // 6. Depth comparison (using shadow bias to prevent shadow acne)
    // Returns 0.0 if in shadow, 1.0 if not in shadow
    float shadow = currentDepth - bias > closestDepth ? 0.0f : 1.0f;

    return shadow;
}

/**
 * Calculates if a fragment is in shadow using standard non-linear depth values.
 * This version works with the default OpenGL depth generation (no custom frag shader).
 * * @param fragPosLightSpace Position of the fragment in the light's clip space (for the specific face).
 * @param cubeLayer The index of the light in the CubeMap Array.
 * @param direction Vector from light to fragment (to sample the correct CubeMap face).
 * @param bias Small offset to prevent shadow acne.
 */
float CalculateCubeShadow(vec4 fragPosLightSpace, uint cubeLayer, vec3 direction, float bias) {
    // 1. Perspective divide to get NDC coordinates [-1, 1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // 2. Map Z from [-1, 1] to [0, 1] range to match depth buffer storage
    float currentDepth = projCoords.z * 0.5 + 0.5;

    // 3. Sample the non-linear depth stored in the CubeMap Array
    // We use the 'direction' vector to pick the face, and 'cubeLayer' for the array index
    float closestDepth = texture(lightCubeShadowMapArray, vec4(direction, float(cubeLayer))).r;

    // 4. Boundary check: if beyond far plane, it's not in shadow
    if (currentDepth > 1.0) return 1.0;

    // 5. Standard depth comparison
    // Since this is non-linear, a very small constant bias might still cause issues 
    // at different distances, but it's the standard approach for non-linear maps.
    float shadow = (currentDepth - bias > closestDepth) ? 0.0 : 1.0;

    return shadow;
}

struct LightCalculateContributionParams {
    Light light;
    vec3 position;      // fragment position
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
    int flags = int(params.light.flags_angle_plane_shadow.x);
    
    // 1. Determine Light Direction and Attenuation
    if ((LIGHT_FLAG_IS_DIR & flags) != 0u) {
        lightDir = normalize(-params.light.direction_lightSpace.xyz);
    }
    else { // Point or Spot Light
        vec3 fragToLight = params.light.position.xyz - params.position;
        float dist = length(fragToLight);
        lightDir = normalize(fragToLight);

        // Attenuation calculation (constant, linear, quadratic)
        attenuation = 1.0 / (params.light.La_const.w + params.light.Ld_linear.w * dist + params.light.Ls_quadratic.w * dist * dist);
        
        if ((LIGHT_FLAG_IS_SPOT & flags) != 0u) {
            // Spot Light Calculation
            vec3 spotDir = normalize(params.light.direction_lightSpace.xyz);
            float theta = dot(lightDir, -spotDir);      // cosine of angle between light ray and spot direction

            float innerCutOff = cos(params.light.flags_angle_plane_shadow.y);
            float outerCutOff = cos(params.light.flags_angle_plane_shadow.z);

            if (theta > outerCutOff) {
                // Smooth fade from inner to outer cutoff (soft edges)
                spotIntensity = smoothstep(outerCutOff, innerCutOff, theta);
            } else {
                // Fragment is outside the spot cone
                spotIntensity = 0.0;
            }
        }
    }

    // If the light is dimmed out by spot or attenuation, skip the expensive calculations
    if (attenuation <= 0.0 || spotIntensity <= 0.0) {
        return vec3(0.0);
    }

    // Calculate shadow
    float shadow = 1.0f;
    if(dot(params.norm, lightDir) <= 0) {
        shadow = 0.f;
    }
    else if ((LIGHT_FLAG_CASTS_SHADOW & flags) != 0u) {
        float bias = max(0.05 * (1.0 - dot(params.norm, lightDir)), 0.005);
        uint layer = uint(params.light.flags_angle_plane_shadow.w);
        
        if ((LIGHT_FLAG_IS_POINT & flags) != 0) {
            // Spot light -> Cube shadow map
            float farPlane = params.light.flags_angle_plane_shadow.z;
            vec3 lightToFrag = params.position - params.light.position.xyz;

            int faceIndex = GetCubeFaceIndex(lightToFrag);
            mat4 viewProj = lightSpaceMatrices[int(params.light.direction_lightSpace.w) + faceIndex];

            shadow = CalculateCubeShadow(viewProj * vec4(params.position, 1), layer, lightToFrag, bias);
        } else {
            // Directional or Spot light -> 2D shadow map array
            mat4 viewProj = lightSpaceMatrices[int(params.light.direction_lightSpace.w)];
            shadow = Calculate2DShadow(viewProj * vec4(params.position, 1), layer, bias);
        }
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