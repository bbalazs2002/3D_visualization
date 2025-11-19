#version 430 core

// SSBO bound to binding point 1
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];   // flexible array member
};

uniform mat4 viewProj;
uniform int ctrlPointCount = 3;
uniform int division = 500;

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

vec3 Bezier(float t) {
    vec3 val = vec3(0);
    int n = ctrlPointCount - 1;
    for (int k = 0; k <= n; ++k) {
        val += BernsteinBase(n, k, t) * ctrlPoints[k].xyz;
    }
    return val;
}

void main()
{
    int index = gl_VertexID;
    int div = division;
    float deltaT = 1.f / float(div - 1);
	gl_Position = viewProj * vec4(Bezier(index * deltaT), 1);
}