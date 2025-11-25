struct Light {
	vec4 La_const;			// xyz: La, w: constant attenuation
	vec4 Ld_linear;			// xyz: Ld, w: linear attenuation
	vec4 Ls_quadratic;		// xyz: Ls, w: quadratic attenuation
	vec4 direction;			// xyz: direction, w: padding
	vec4 position;			// xyz: position, w: padding
	vec4 type_angle;		// x: type, y: inner angle, z: outer angle, w: padding
};