#include "Math.glsl"

float BernsteinBase(int n, int k, float t) {
    return binomialCoeff(n, k) * ipow(1.f - t, n - k) * ipow(t, k);
}

struct BezierParams {
    float t;
    int ctrlPointCount;
    vec4 ctrlPoints[];
};

vec3 Bezier(BezierParams p) {
    vec3 val = vec3(0);
    int n = p.ctrlPointCount - 1;
    for (int k = 0; k <= n; ++k) {
        val += BernsteinBase(n, k, p.t) * p.ctrlPoints[k].xyz;
    }
    return val;
}