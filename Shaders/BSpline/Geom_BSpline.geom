#version 430 core

// === Control points SSBO ===
layout(std430, binding = 1) buffer ctrlPointsSSBO {
    vec4 ctrlPoints[];
};

// === Knot vector SSBO ===
layout(std430, binding = 2) buffer knotSSBO {
    float knots[];
};

// Bemenet: pontok
layout(points) in;
// Kimenet: vonalcsík
layout(line_strip, max_vertices = 64) out;

uniform mat4 viewProj;
uniform int ctrlPointCount = 4; // Kontrollpontok száma
uniform int knotCount = 8;      // Csomópontok száma
uniform int degree = 3;         // Fokszám
uniform int division = 50;      // Kiértékelési lépések
uniform vec3 color;

// --- Robusztus lebegõpontos összehasonlításhoz ---
const float EPSILON = 0.00001;
const int MAX_CTRL_POINTS = 64; 

// -------------------------------------------------------------
// Iteratív Cox–de Boor basis function (robuzstus végpont kezeléssel)
// -------------------------------------------------------------
float BSplineBasis(int i, int p, float t)
{
    float N[MAX_CTRL_POINTS];
    float Nprev[MAX_CTRL_POINTS];
    
    // Szükséges tEnd érték a végpont ellenõrzéséhez
    float tEnd = knots[knotCount - p - 1]; 
    
    // --- Alapeset: p=0 (a k=0 fokszám) ---
    for (int j = 0; j < ctrlPointCount; ++j) {
        // Alap feltétel: zárt bal, nyitott jobb intervallum
        bool condition = (t >= knots[j] && t < knots[j + 1]);
        
        // KRITIKUS JAVÍTÁS: Epsilon használata az egyenlõség vizsgálatához!
        // Ha t NAGYON KÖZEL van a tEnd-hez, ÉS ez a csomópont (knots[j+1]) a tEnd, 
        // akkor az intervallum jobb oldalát is zártnak vesszük.
        if (abs(t - tEnd) < EPSILON && abs(knots[j + 1] - tEnd) < EPSILON) {
            condition = (t >= knots[j] && t <= knots[j + 1] + EPSILON); 
            // A <= operátort is lazítjuk EPSILON-nal
        }
        
        Nprev[j] = condition ? 1.0 : 0.0;
    }

    // --- Felépítés k=1-tõl p-ig (a fokszámig) ---
    for (int k = 1; k <= p; ++k) {
        for (int j = 0; j < ctrlPointCount; ++j) {
            float left_term = 0.0;
            float right_term = 0.0;

            // Elsõ tag (Left)
            float denom1 = knots[j + k] - knots[j];
            if (denom1 > EPSILON) { // Ellenõrzés: ha a nevezõ nem 0 (vagy annak közelében van)
                left_term = ((t - knots[j]) / denom1) * Nprev[j];
            }

            // Második tag (Right)
            float denom2 = knots[j + k + 1] - knots[j + 1];
            if (denom2 > EPSILON) { // Ellenõrzés: ha a nevezõ nem 0
                 right_term = ((knots[j + k + 1] - t) / denom2) * Nprev[j + 1];
            }

            N[j] = left_term + right_term;
        }

        // Másolás a következõ iterációhoz
        for (int j = 0; j < ctrlPointCount; ++j) {
            Nprev[j] = N[j];
        }
    }

    // A B-spline definíció szerint a bázisfüggvények összegének 1.0-nak kell lennie a kiértékelési tartományban.
    // Ezt az ellenõrzést (vagy a de Boor algoritmussal történõ kiértékelést) célszerû használni a robusztusabb megoldásoknál.
    return Nprev[i];
}

// -------------------------------------------------------------
// Evaluate B-spline curve point at t
// -------------------------------------------------------------
vec3 BSpline(float t)
{
    vec3 C = vec3(0);
    for (int i = 0; i < ctrlPointCount; ++i) {
        float w = BSplineBasis(i, degree, t);
        C += w * ctrlPoints[i].xyz;
    }
    return C;
}

// -------------------------------------------------------------
// Determine valid t range [u_p, u_{m-p}]
// -------------------------------------------------------------
float GetTStart() {
    return knots[degree];
}
float GetTEnd() {
    return knots[knotCount - degree - 1];
}

// -------------------------------------------------------------
// Main geometry shader
// -------------------------------------------------------------
void main() {
    int div = max(2, min(64, division)); 
    float tStart = GetTStart();
    float tEnd = GetTEnd();
    float deltaT = (tEnd - tStart) / float(div - 1);

    for (int i = 0; i < div; ++i) {
        float t = tStart + float(i) * deltaT;
        
        // A lebegõpontos problémák elkerülése végett továbbra is javasolt a végpont fixálása,
        // így garantált, hogy a tEnd értéke pontosan megegyezik a BSplineBasis-ben lévõ tEnd-del.
        if (i == div - 1) {
            t = tEnd; 
        }

        gl_Position = viewProj * vec4(BSpline(t), 1);
        EmitVertex();
    }
    EndPrimitive();
}
