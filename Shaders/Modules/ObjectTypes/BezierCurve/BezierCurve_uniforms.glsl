#ifndef BEZIER_CURVE_CTRL_POINTS_SSBO
	#error "BEZIER_CURVE_CTRL_POINTS_SSBO macro is undefined"
#endif

layout(std430, binding = BEZIER_CURVE_CTRL_POINTS_SSBO) buffer BezierCurveCtrlPointsBuffer {
    vec4 bezierCurveCtrlPoints[];
};

struct BezierCurveUniforms{
	int ctrlPointCount;
	int division;
};
uniform BezierCurveUniforms bezierCurveData;