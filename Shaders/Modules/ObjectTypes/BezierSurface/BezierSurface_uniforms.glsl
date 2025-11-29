#ifndef BEZIER_SURFACE_CTRL_POINTS_SSBO
    #error "BEZIER_SURFACE_CTRL_POINTS_SSBO macro is undefined!"
#endif

layout(std430, binding = BEZIER_SURFACE_CTRL_POINTS_SSBO) readonly buffer BezierSurfaceCtrlPointsSSBO {
    vec4 bezierSurfaceCtrlPoints[];
};

struct BezierSurfaceUniforms{
    ivec2 ctrlPointCount;
    ivec2 division;
};
uniform BezierSurfaceUniforms bezierSurfaceData;