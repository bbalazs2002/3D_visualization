#ifndef LIGHT_LIGHTS_SSBO
	#error "LIGHT_LIGHTS_SSBO macro is undefined!"
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
	mat4 lightSpaceMatrix;		//
	vec4 La_const;				// xyz: La, w: constant attenuation
	vec4 Ld_linear;				// xyz: Ld, w: linear attenuation
	vec4 Ls_quadratic;			// xyz: Ls, w: quadratic attenuation
	vec4 direction;				// xyz: direction, w: padding
	vec4 position;				// xyz: position, w: padding
	vec4 flags_angle_shadow;	// x: flags, y: inner angle, z: outer angle, w: shadowLayer
};
uniform sampler2DArray lightShadowMapArray;

layout(std430, binding = LIGHT_LIGHTS_SSBO) buffer LightBuffer {
	Light lightSources[];
};

struct LightUniforms{
	int lightCount;
};
uniform LightUniforms lightData;