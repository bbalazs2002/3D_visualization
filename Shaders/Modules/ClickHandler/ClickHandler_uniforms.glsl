// SSBO setter (layout + binding is in the shader)
#ifndef CLICK_HANDLER_SET_SSBO
	#error "CLICK_HANDLER_SET_SSBO(i, val) macro is undefined!"
#endif

struct ClickHandlerUniforms {
	ivec2 windowSize = ivec2(0,0,);
	vec2 cursorPos = ivec2(0,0);
	int modelID = -1;
};
uniform ClickHandlerUniforms clickHandlerData;