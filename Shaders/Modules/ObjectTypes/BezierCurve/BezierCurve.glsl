#include "../../Math.glsl"

float BernsteinBase(int n, int k, float t) {
    return binomialCoeff(n, k) * ipow(1.f - t, n - k) * ipow(t, k);
}

struct BezierParams {
    float t;
    int ctrlPointCount;
};
vec3 BezierCurve(BezierParams params) {
    vec3 val = vec3(0);
    int n = params.ctrlPointCount - 1;
    for (int k = 0; k <= n; ++k) {
        val += BernsteinBase(n, k, params.t) * bezierCurveCtrlPoints[k].xyz;
    }
    return val;
}