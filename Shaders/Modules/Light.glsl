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