#ifndef SET_CURRENT_MODEL_ID
	#error "SET_CURRENT_MODEL_ID(i, val) macro is undefined!"
#endif

struct ClickHandlerParams {
	ivec2 windowSize;
	ivec2 cursorPos;
	ivec2 fragPos;
	int modelID;
};

void ClickHandler(ClickHandlerParams params) {
    ivec2 cursor = ivec2(params.cursorPos.x, params.windowSize.y - params.cursorPos.y);
	// index is 0 if frag == cursor, 1 if frag != cursor
	// save model below cursor to index 0
	SET_CURRENT_MODEL_ID(1 - int(params.fragPos == cursor), vec4(params.modelID));
}