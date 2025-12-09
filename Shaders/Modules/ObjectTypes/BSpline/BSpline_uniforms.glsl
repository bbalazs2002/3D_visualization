#ifndef BSPLINE_CTRL_POINTS_SSBO
    #error "BSPLINE_CTRL_POINTS_SSBO macro is udefined!"
#endif
#ifndef BSPLINE_KNOTS_SSBO
    #error "BSPLINE_KNOTS_SSBO macro is udefined!"
#endif

// === Control points SSBO ===
layout(std430, binding = BSPLINE_CTRL_POINTS_SSBO) buffer BSplineCtrlPointsSSBO {
    vec4 BSplineCtrlPoints[];
};

// === Knot vector SSBO ===
layout(std430, binding = BSPLINE_KNOTS_SSBO) buffer BSplineKnotSSBO {
    float BSplineKnots[];
};

struct BSplineUniforms {
    int degree;             // degree of a single curve
    int knotCount;          // number of knots
    int ctrlPointCount;     // number of control points
    int division;           // total number of divisions for the whole B-Spline object
};
uniform BSplineUniforms bSplineData;