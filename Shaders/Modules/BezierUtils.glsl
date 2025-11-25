#include "Math.glsl"

#ifndef BEZIER_GET_CTRL_POINT
    #error "BEZIER_GET_CTRL_POINT(i) macro is undefined!"
#endif

float BernsteinBase(int n, int k, float t) {
    return binomialCoeff(n, k) * ipow(1.f - t, n - k) * ipow(t, k);
}

struct BezierParams {
    float t;
    int ctrlPointCount;
};

vec3 Bezier(BezierParams params) {
    vec3 val = vec3(0);
    int n = params.ctrlPointCount - 1;
    for (int k = 0; k <= n; ++k) {
        val += BernsteinBase(n, k, params.t) * BEZIER_GET_CTRL_POINT(k).xyz;
    }
    return val;
}