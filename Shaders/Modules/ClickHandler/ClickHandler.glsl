void ClickHandler() {
	ivec2 cursor = ivec2(
		clickHandlerData.cursorPos.x,
		clickHandlerData.windowSize.y - clickHandlerData.cursorPos.y
	);

	// index is 0 if frag == cursor, 1 if frag != cursor
	// save model below cursor to index 0
	clickHandlerModel[1 - int(ivec2(gl_FragCoord.xy) == cursor)] = vec4(clickHandlerData.modelID);
}