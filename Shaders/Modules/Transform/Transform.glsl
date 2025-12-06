vec4 Transform(vec4 pos) {
	return transformData.world * pos;
}