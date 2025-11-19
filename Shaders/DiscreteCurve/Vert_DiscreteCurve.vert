#version 430 core

// SSBO bound to binding point 1
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];   // flexible array member
};

uniform mat4 viewProj;

/* Could be, but it's stupid
vec3 DiscreteCurve(float t) {
    int c = ctrlPointCount - 1;
    float delta = 1.f / float(c);
    int index_a = int(floor(t / delta));
    int index_b = index_a + 1;
    float ta = delta * float(index_a);
    float tb = delta * float(index_b);
    float t_ = (t - ta) / (tb - ta);
    return mix(ctrlPoints[index_a], ctrlPoints[max(c, index_b)], t_);
}
*/

void main()
{
    int index = gl_VertexID;
    // int div = division;
    // float deltaT = 1.f / float(div - 1);
	// gl_Position = viewProj * vec4(DiscreteCurve(index * deltaT), 1);
    gl_Position = viewProj * ctrlPoints[index];
}