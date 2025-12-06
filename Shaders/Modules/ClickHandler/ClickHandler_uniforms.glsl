#ifndef CLICK_HANDLER_MODEL_SSBO
	#error "CLICK_HANDLER_MODEL_SSBO macro is undefined!"
#endif

layout(early_fragment_tests) in;
layout(std430, binding = CLICK_HANDLER_MODEL_SSBO) buffer ClickHandlerModelBuffer {
    vec4 clickHandlerModel[];
};

struct ClickHandlerUniforms {
	ivec2 windowSize;
	ivec2 cursorPos;
	int modelID;
};
uniform ClickHandlerUniforms clickHandlerData;