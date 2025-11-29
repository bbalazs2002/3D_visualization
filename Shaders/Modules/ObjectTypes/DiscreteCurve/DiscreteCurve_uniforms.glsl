#ifndef DISCRETE_CURVE_CTRL_POINTS_SSBO
    #error "DISCRETE_CURVE_CTRL_POINTS_SSBO macro is undefined!"
#endif

layout(std430, binding = DISCRETE_CURVE_CTRL_POINTS_SSBO) buffer DiscreteCurveCtrlPointsSSBO {
    vec4 DiscreteCurveCtrlPoints[];   // flexible array member
};