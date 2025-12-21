#ifndef LIGHT_LIGHTS_SSBO
	#error "LIGHT_LIGHTS_SSBO macro is undefined!"
#endif

#ifndef LIGHT_TYPE_DIRECTIONAL
	#define LIGHT_TYPE_DIRECTIONAL 0
#endif
#ifndef LIGHT_TYPE_POINT
	#define LIGHT_TYPE_POINT 1
#endif
#ifndef LIGHT_TYPE_SPOT
	#define LIGHT_TYPE_SPOT 2
#endif

struct Light {
	vec4 La_const;			// xyz: La, w: constant attenuation
	vec4 Ld_linear;			// xyz: Ld, w: linear attenuation
	vec4 Ls_quadratic;		// xyz: Ls, w: quadratic attenuation
	vec4 direction;			// xyz: direction, w: padding
	vec4 position;			// xyz: position, w: padding
	vec4 type_angle;		// x: flags, y: inner angle, z: outer angle, w: padding
};

layout(std430, binding = LIGHT_LIGHTS_SSBO) buffer LightBuffer {
	Light lightSources[];
};

struct LightUniforms{
	int lightCount;
};
uniform LightUniforms lightData;