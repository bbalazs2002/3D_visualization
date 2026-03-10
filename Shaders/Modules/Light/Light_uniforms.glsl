#ifndef LIGHT_LIGHTS_SSBO
	#error "LIGHT_LIGHTS_SSBO macro is undefined!"
#endif

#ifndef LIGHT_SPACE_MATRICES_SSBO
	#error "LIGHT_SPACE_MATRICES_SSBO macro is undefined!"
#endif

#ifndef LIGHT_FLAG_IS_DIR
	// #error "LIGHT_FLAG_IS_DIR macro is undefined! Default value is used"
	#define LIGHT_FLAG_IS_DIR       (1u << 0) // 1
#endif
#ifndef LIGHT_FLAG_IS_POINT
	// #error "LIGHT_FLAG_IS_POINT macro is undefined! Default value is used"
	#define LIGHT_FLAG_IS_POINT     (1u << 1) // 2
#endif
#ifndef LIGHT_FLAG_IS_SPOT
	// #error "LIGHT_FLAG_IS_SPOT macro is undefined! Default value is used"
	#define LIGHT_FLAG_IS_SPOT      (1u << 2) // 4
#endif
#ifndef LIGHT_FLAG_CASTS_SHADOW
	// #error "LIGHT_FLAG_CASTS_SHADOW macro is undefined! Default value is used"
	#define LIGHT_FLAG_CASTS_SHADOW (1u << 3) // 8
#endif

struct Light {
	vec4 La_const;					// xyz: La, w: constant attenuation
	vec4 Ld_linear;					// xyz: Ld, w: linear attenuation
	vec4 Ls_quadratic;				// xyz: Ls, w: quadratic attenuation
	vec4 direction_lightSpace;		// xyz: direction, w: lightSpaceMatrix
	vec4 position;					// xyz: position, w: padding
	vec4 flags_angle_plane_shadow;	// x: flags, y: inner angle (for spot), z: outer angle (for spot), w: shadowLayer
									//			 y: near plane (for point), z: far plane (for point), 
};
uniform sampler2DArray light2DShadowMapArray;
uniform samplerCubeArray lightCubeShadowMapArray;

layout(std430, binding = LIGHT_LIGHTS_SSBO) buffer LightBuffer {
	Light lightSources[];
};

layout(std430, binding = LIGHT_SPACE_MATRICES_SSBO) buffer LightSpaceMatricesSSBO {
	mat4 lightSpaceMatrices[];
};

struct LightUniforms{
	int lightCount;
};
uniform LightUniforms lightData;