#version 430 core

// SSBO bound to binding point 1
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];   // flexible array member
};

// out vec3 vs_out_col;
out vec3 vs_out_pos;
out vec3 vs_out_norm;
out vec2 vs_out_tex;

uniform mat4 viewProj;
uniform ivec2 ctrlPointCount = ivec2(5, 5);
uniform ivec2 division = ivec2(10,10);

float binomialCoeff(int n, int k) {
    float result = 1.0;
    for (int i = 1; i <= k; i++) {
        result *= float(n - (k - i)) / float(i);
    }
    return result;
}

float ipow(float base, int exponent) {
    float result = 1.0;
    for (int i = 0; i < exponent; i++)
        result *= base;
    return result;
}

float BernsteinBase(int n, int k, float t) {
    return binomialCoeff(n, k) * ipow(1.f - t, n - k) * ipow(t, k);
}

/**
 * @brief Calculates the first derivative of the Bernstein basis function.
 * @details B'_n,k(t) = n * (B_{n-1, k-1}(t) - B_{n-1, k}(t)), where B_{m,l}(t) = 0 if l < 0 or l > m.
 */
float BernsteinBaseDerivative(int n, int k, float t) {
    if (n == 0) return 0.0;
    
    float B_km1; // B_{n-1, k-1}(t)
    if (k - 1 < 0) {
        B_km1 = 0.0;
    } else {
        B_km1 = BernsteinBase(n - 1, k - 1, t);
    }

    float B_k; // B_{n-1, k}(t)
    if (k > n - 1) {
        B_k = 0.0;
    } else {
        B_k = BernsteinBase(n - 1, k, t);
    }

    return float(n) * (B_km1 - B_k);
}

// optimized version of BernsteinBaseDerivative
float BernsteinBaseDerivative_SIMD(int n, int k, float t) {
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
    float mask_k = step(float(n - 1), float(k)) == 0.0 ? 1.0 : 0.0; 
    // Alternative mask that might be better for SIMD, though less direct:
    // float mask_k = 1.0 - step(float(n), float(k));

    float B_k = B_k_value * mask_k;

    // B'_n,k(t) = n * (B_{n-1, k-1}(t) - B_{n-1, k}(t))
    return float(n) * (B_km1 - B_k);
}

vec3 BSurface(float u, float v) {
    // u = min(1.f, max(0, u));
    // v = min(1.f, max(0, v));
    vec3 p = vec3(0, 0, 0);
    for (int i = 0; i < ctrlPointCount.x; ++i) {          // sor (v)
        vec3 p_ = vec3(0.0);
        for (int j = 0; j < ctrlPointCount.y; ++j) {      // oszlop (u)
            int index = i * int(ctrlPointCount.y) + j;    // row-major
            p_ += BernsteinBase(int(ctrlPointCount.y) - 1, j, u) * ctrlPoints[index].xyz;
        }
        p += BernsteinBase(int(ctrlPointCount.x) - 1, i, v) * p_;
    }
    return p;
}

/**
 * @brief Calculates the partial derivative of the Bezier surface with respect to u.
 */
vec3 BSurface_du(float u, float v) {
    vec3 p_u = vec3(0, 0, 0);
    int n = ctrlPointCount.x - 1; // Degree in v direction
    int m = ctrlPointCount.y - 1; // Degree in u direction

    for (int i = 0; i <= n; ++i) {      // v direction (row)
        vec3 p_ = vec3(0.0);
        for (int j = 0; j <= m; ++j) {   // u direction (column)
            int index = i * (m + 1) + j; // row-major
            p_ += BernsteinBaseDerivative(m, j, u) * ctrlPoints[index].xyz;
        }
        p_u += BernsteinBase(n, i, v) * p_;
    }
    return p_u;
}

/**
 * @brief Calculates the partial derivative of the Bezier surface with respect to v.
 */
vec3 BSurface_dv(float u, float v) {
    vec3 p_v = vec3(0, 0, 0);
    int n = ctrlPointCount.x - 1; // Degree in v direction
    int m = ctrlPointCount.y - 1; // Degree in u direction

    for (int i = 0; i <= n; ++i) {      // v direction (row)
        vec3 p_ = vec3(0.0);
        for (int j = 0; j <= m; ++j) {   // u direction (column)
            int index = i * (m + 1) + j; // row-major
            p_ += BernsteinBase(m, j, u) * ctrlPoints[index].xyz;
        }
        p_v += BernsteinBaseDerivative(n, i, v) * p_;
    }
    return p_v;
}

// Point cloud
/*
void main() {

    int index = gl_VertexID;

    int u = index % division.x;
    int v = int(floor(float(index) / float(division.x)));

    int divu = int(division.x);
    float deltau = 1.f / float(divu - 1);

    int divv = int(division.y);
    float deltav = 1.f / float(divv - 1);
    
    vec4 p = vec4(BSurface(u * deltau, v * deltav), 1);
    gl_Position = viewProj * p;
}
*/

// Triangles
void main()
{
    int divu = int(division.x);
    float deltau = 1.f / float(divu - 1);
    int divv = int(division.y);
    float deltav = 1.f / float(divv - 1);

    int tid = int(floor(gl_VertexID / 3));  // id of the current triangle

    int vr = (division.x - 1) * 2 * 3;      // verteces in a row
    int row = int(floor(gl_VertexID / vr)); // id of the current row
    int correctID = gl_VertexID - vr * row; // collapse to one row
    int col = int(floor(correctID / 6));    // id of the current column
    correctID = correctID - col * 6;        // collapse to one column

    float u = col * deltau
            + (tid % 2) * (1 - int(floor(correctID / 5))) * deltau           // +w if the correctID is 3 or 4
            + (1 - (tid % 2)) * int(floor(correctID / 2)) * deltau;          // +w if the correctID is 2
    float v = row * deltav
            + (gl_VertexID % 2) * deltav;                                    // +h if the ID is odd

    vs_out_tex = vec2(u,v);

    vec4 p = vec4(BSurface(u, v), 1);
    gl_Position = viewProj * p;
    vs_out_pos = (viewProj * p).xyz;

    vec3 T_u = BSurface_du(u, v);
    vec3 T_v = BSurface_dv(u, v);
    vs_out_norm = normalize(cross(T_v, T_u));
}

// Triangle strip
/*
void main()
{
    // TRIANGLE_STRIP
    float w = 1.f / division.x;
    float h = 1.f / division.y;

    int period = 2 + cols * 4;
    int blockID = int(floor(gl_VertexID / period));
    int correctID = gl_VertexID - blockID * period;

    int sgn = int(round(max(0.0, sign(period / 2.0 - correctID))));       // (-inf; 0] => 0, (0; +inf) => 1

    float x = position.x
        + sgn * ((correctID - (correctID % 2)) / 2.0) * w                                     // position if the vertex is in the upper row of the block
        + (1 - sgn) * (cols - ((correctID + (correctID % 2)) / 2.0 - (cols + 1.0))) * w;      // position if the vertex is in the lower row of the block

    float z = position.z
        + h * (
            blockID * 2.0                                   // number of rows before this block
            + sgn * (correctID % 2)                         // relative position to the block for the upper row
            + (1 - sgn) * (1.0 + (1.0 - (correctID % 2)))   // relative position to the block for the lower row
        );
  
    vec2 texPos = vec2((x - position.x) / width, (z - position.z) / height);
    vs_out_color = texture(texImage, texPos);
    // vs_out_color = vec4(.0, 1., .0, 1.);
    // vec4 hTex = texture(hightMapImage, texPos);
    gl_Position = viewProj * world * vec4(x, position.y + texture(hightMapImage, texPos).g * heightMult, z, 1.0);
    // gl_Position = viewProj * world * vec4(x, 0.0, z, 1.0);
}
*/