vec4[] bezierCurveCtrlPoints = vec4[](vec4(1));
#include "../BezierCurve/BezierCurve.glsl"

float BernsteinBaseDerivative(int n, int k, float t) {
    // If degree n is 0, the derivative is always 0.
    if (n == 0) {
        return 0.0;
    }

    // Term 1: B_{n-1, k-1}(t)
    // This term is non-zero only if k-1 >= 0, or k >= 1.
    float B_km1_value = BernsteinBase(n - 1, k - 1, t);
    // Mask: 1.0 if k >= 1, 0.0 otherwise.
    float mask_km1 = step(1.0, float(k)); 
    float B_km1 = B_km1_value * mask_km1;

    // Term 2: B_{n-1, k}(t)
    // This term is non-zero only if k <= n-1.
    float B_k_value = BernsteinBase(n - 1, k, t);
    // Mask: 1.0 if k <= n-1, 0.0 otherwise.
    // float mask_k = step(float(n - 1), float(k)) == 0.0 ? 1.0 : 0.0; 
    // Alternative mask that might be better for SIMD, though less direct:
    float mask_k = 1.0 - step(float(n), float(k));

    float B_k = B_k_value * mask_k;

    // B'_n,k(t) = n * (B_{n-1, k-1}(t) - B_{n-1, k}(t))
    return float(n) * (B_km1 - B_k);
}

struct BezierSurfaceParams{
    float u;
    float v;
    ivec2 ctrlPointCount;
};

/**
 * @brief Calculates the partial derivative of the Bezier surface with respect to u.
 */
vec3 BezierSurface_du(BezierSurfaceParams params) {
    vec3 p_u = vec3(0, 0, 0);
    int n = params.ctrlPointCount.x - 1; // Degree in v direction
    int m = params.ctrlPointCount.y - 1; // Degree in u direction

    for (int i = 0; i <= n; ++i) {      // v direction (row)
        vec3 p_ = vec3(0.0);
        for (int j = 0; j <= m; ++j) {   // u direction (column)
            int index = i * (m + 1) + j; // row-major
            p_ += BernsteinBaseDerivative(m, j, params.u) * bezierSurfaceCtrlPoints[index].xyz;
        }
        p_u += BernsteinBase(n, i, params.v) * p_;
    }
    return p_u;
}

/**
 * @brief Calculates the partial derivative of the Bezier surface with respect to v.
 */
vec3 BezierSurface_dv(BezierSurfaceParams params) {
    vec3 p_v = vec3(0, 0, 0);
    int n = params.ctrlPointCount.x - 1; // Degree in v direction
    int m = params.ctrlPointCount.y - 1; // Degree in u direction

    for (int i = 0; i <= n; ++i) {      // v direction (row)
        vec3 p_ = vec3(0.0);
        for (int j = 0; j <= m; ++j) {   // u direction (column)
            int index = i * (m + 1) + j; // row-major
            p_ += BernsteinBase(m, j, params.u) * bezierSurfaceCtrlPoints[index].xyz;
        }
        p_v += BernsteinBaseDerivative(n, i, params.v) * p_;
    }
    return p_v;
}

vec3 BezierSurface(BezierSurfaceParams params) {
    // u = min(1.f, max(0, u));
    // v = min(1.f, max(0, v));
    vec3 p = vec3(0, 0, 0);
    for (int i = 0; i < params.ctrlPointCount.x; ++i) {          // sor (v)
        vec3 p_ = vec3(0.0);
        for (int j = 0; j < params.ctrlPointCount.y; ++j) {      // oszlop (u)
            int index = i * int(params.ctrlPointCount.y) + j;    // row-major
            p_ += BernsteinBase(int(params.ctrlPointCount.y) - 1, j, params.u) * bezierSurfaceCtrlPoints[index].xyz;
        }
        p += BernsteinBase(int(params.ctrlPointCount.x) - 1, i, params.v) * p_;
    }
    return p;
}

vec3 BezierSurfaceNormal(BezierSurfaceParams params) {
    vec3 T_u = BezierSurface_du(params);
    vec3 T_v = BezierSurface_dv(params);
    return normalize(cross(T_v, T_u));
}