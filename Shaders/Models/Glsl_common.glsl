// debug buffers
layout(std430, binding = 0) buffer VisDebug{
    vec4 vdbug[];                                 // buffer for visual debug
};

layout(std430, binding = 1) buffer NumDebug{
    vec4 ndbug[];                                // buffer for numerical debug
};

// uniforms
uniform sampler2D coneMap;
uniform int maxSteps = 50;

// structs
struct UnitIntersection{
    bool found;
    float near;
    float far;
};
struct Ray{
    vec3 start;
    vec3 dir;
};
struct IntersectParams{
    vec3 enter;
    vec3 exit;
    vec3 cam;
    int vDebugStart;
};
struct IntersectReturn{
    vec2 uv;
    float t;
    float last_t;
    bool wasHit;

    int flags;
    int stepCount;
};
struct StepParams {
    vec3 point;
    Ray view;
    vec2 tex;
};
struct StepReturn{
    float t;
};

#ifdef GEOMETRY_SHADER
// debug functionality for the geometry shader
void visualDebugSet(int index, vec4 data) {
    vdbug[index] = data;
}
void visualDebugSet(int index, mat4x4 data) {
    for (int i = 0; i < 4; ++i) {
        vdbug[index + i] = data[i];
    }
}
vec4 visualDebugGet(int index) {
    return vdbug[index];
}
void numericalDebugSet(int index, vec4 data) {
    ndbug[index] = data;
}
void numericalDebugSet(int index, mat4x4 data) {
    for (int i = 0; i < 4; ++i) {
        ndbug[index + i] = data[i];
    }
}
vec4 numericalDebugGet(int index) {
    return ndbug[index];
}
#else
// debug is not needed in the fragment shader
void visualDebugSet(int index, vec4 data) {}
void visualDebugSet(int index, mat4x4 data) {}
vec4 visualDebugGet(int index) {return vec4(0);}
void numericalDebugSet(int index, vec4 data) {}
void numericalDebugSet(int index, mat4x4 data) {}
vec4 numericalDebugGet(int index) {return vec4(0);}
#endif

// conemap texture handling
vec2 conemap_get(vec2 uv) {
    return texture(coneMap, uv);    // .r is the height; .g is the tangent of the cone
}
float conemap_getHeight(vec2 uv) {
    return conemap_get(uv).r;
}

StepReturn getNextStep(StepParams params) {       // current intersection point, view vector (u1 -> u2)

    vec3 u = params.point;
    vec3 e = params.view.start;
    vec3 v = params.view.dir;

    StepReturn val;

    float az = params.tex.r;            // height
    float ctga = 1.f / params.tex.g;    // 1 / tg
    float sq = ((e.y - u.y) / (e.x - u.x)) * ((e.y - u.y) / (e.x - u.x));
    float gamma = sqrt(1.f + sq);
    float t1 = (az - u.z) / (v.z - gamma * v.x * ctga);
    float t2 = (az - u.z) / (v.z + gamma * v.x * ctga);

    val.t = max(t1, t2);
    return val;
}
IntersectReturn findIntersection_coneStepMapping(IntersectParams params) {
    vec3 u1 = params.enter;
    vec3 u2 = params.exit;
    int stepCount = 0;
    int flags = 0;

    // pre-check
    if (conemap_getHeight(u1.xy) >= u1.z) {

        visualDebugSet(params.vDebugStart, vec4(u1, 1.));

        flags = int(false) |
            (int(false) << 1) |
            (int(true)  << 2);

        numericalDebugSet(0, vec4(stepCount, 0, 0, 0));
        return IntersectReturn(
            u1.xy,          // uv
            0.f, 0.f,       // t, last_t
            true,           // wasHit
            0, 0            // flags, stepCount
        );
    }

    vec3 v = u2 - u1;       // direction vector from u1 to u2

    numericalDebugSet(15, vec4(v, 0));

    float maxT = 1.;        // t parameter of the exit point (u2)

    vec3 ui = u1 + 0.000001f * v;
    vec2 tex = conemap_get(ui.xy);
    float t = 0.000001f;                    // t parameter of the intersection point (u1 -> ui)
    float last_t = 0.f;
    float ti = t + 1.f;                     // t parameter of the current step (ui -> ui+1)

    while(
        stepCount <= maxSteps &&            // max step count reached => divergent
        t < maxT &&       	                // Stay within prism
        ti > 0.000001f * t                  // Stop if cone is close to surface
    ) {
        StepReturn stepData = getNextStep(StepParams(ui, Ray(u1, v), tex));
        ti = stepData.t;
        last_t = t;
        t += ti;
        ui = u1 + t * v;
        tex = conemap_get(ui.xy);

        visualDebugSet(params.vDebugStart + stepCount, vec4(ui, 1));
        numericalDebugSet(16 + stepCount * 2, vec4(ti, t, tex));
        numericalDebugSet(16 + stepCount * 2 + 1, vec4(ui, 0));

        if (ui.z <= tex.r) {    // intersection is below the surface
            ti = -1.f;
            break;
        }

        ++stepCount;
    }

    flags = int(stepCount > maxSteps) |
            (int(t >= maxT) << 1) |
            (int(ti <= 0.000001 * t)  << 2);

    return IntersectReturn(
        ui.xy,
        t, last_t,
        bool(flags & 4) && !bool(flags ^ 4),
        flags, stepCount
    );
}

// unit prism intersection
UnitIntersection intersectUnitPrism(Ray ray) {     // ray in unit prism space
    float n = -1e10, f = 1e10; // near, far

    vec3 p0 = ray.start;
    vec3 v = ray.dir;

    vec3 t0 = -p0 / v; // a solution for each cardinal normal
    if (v.x > 0.) { n = max(n, t0.x); } else { f = min(f, t0.x); }
    if (v.y > 0.) { n = max(n, t0.y); } else { f = min(f, t0.y); }
    if (v.z > 0.) { n = max(n, t0.z); } else { f = min(f, t0.z); }

    vec3 q = vec3(1, 1, 0) - p0;
    float t1 = q.y / v.y;
    if (v.y < 0.) { n = max(n, t1); } else { f = min(f, t1); }
    float t2 = (q.x + q.z) / (v.x + v.z);
    if (v.x + v.z < 0.) { n = max(n, t2); } else { f = min(f, t2); }

    return UnitIntersection(n < f, n, f);
}