#include "ClickHandler_uniforms.glsl"

struct ClickHandlerParams {
	ivec2 windowSize;
	ivec2 cursorPos;
	ivec2 fragPos;
	int modelID;
};

void ClickHandler() {
	ivec2 cursor = ivec2(
		clickHandlerData.cursorPos.x,
		clickHandlerData.windowSize.y - clickHandlerData.cursorPos.y
	);

	// index is 0 if frag == cursor, 1 if frag != cursor
	// save model below cursor to index 0
	SET_CURRENT_MODEL_ID(
		1 - int(clickHandlerData.fragPos == cursor),
		vec4(clickHandlerData.modelID)
	);
}