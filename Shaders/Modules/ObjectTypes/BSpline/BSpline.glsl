// Konstansok
const float BSPLINE_EPSILON = 0.00001;

// Maximum fokszám a helyi tömbökhöz. 
// A B-Spline fokszáma ritkán nagyobb 3-nál vagy 5-nél, a 10 bõségesen elég.
const int BSPLINE_MAX_DEGREE = 10;

struct BSplineParams {
    int degree;
    float t;
    int knotCount;
    int ctrlPointCount;
};

struct GetTParams {
    int degree;
    int knotCount;
};

// -------------------------------------------------------------
// Segédfüggvények a tartomány lekéréséhez
// -------------------------------------------------------------
float BSplineGetTStart(GetTParams params) {
    return BSplineKnots[params.degree];
}

float BSplineGetTEnd(GetTParams params) {
    return BSplineKnots[params.knotCount - params.degree - 1];
}

// -------------------------------------------------------------
// 1. Lépés: Knot Span keresése
// Megkeresi azt az 'i' indexet, amelyre: u_i <= t < u_{i+1}
// Ez a bináris keresés sokkal gyorsabb, mint a lineáris végigiterálás.
// -------------------------------------------------------------
int BSplineFindKnotSpan(BSplineParams params) {
    // Ha t == tEnd, akkor az utolsó érvényes intervallumot adjuk vissza
    // (A "knotCount - degree - 1" az utolsó csomópont indexe a tartományban)
    int n = params.knotCount - params.degree - 1; // Ez felel meg az m-p-nek a szakirodalomban
    
    float tEnd = BSplineKnots[n];
    
    if (params.t >= tEnd - BSPLINE_EPSILON) {
        return n - 1;
    }

    // Bináris keresés
    int low = params.degree;
    int high = n;
    int mid = (low + high) / 2;

    // Biztonsági korlát a while ciklushoz (bár a bináris keresés gyorsan konvergál)
    for(int i = 0; i < 64; i++) { 
        float midKnot = BSplineKnots[mid];
        float nextKnot = BSplineKnots[mid + 1];

        if (params.t >= midKnot && params.t < nextKnot) {
            return mid;
        }

        if (params.t < midKnot) {
            high = mid;
        } else {
            low = mid;
        }
        mid = (low + high) / 2;
        
        if(low == mid) { // Ha beragadna (pl. lebegõpontos pontatlanság miatt)
             // Ellenõrzés, hogy a high jó-e
             if(params.t >= BSplineKnots[high] && params.t < BSplineKnots[high+1]) 
                return high;
             return low;
        }
    }
    return low;
}

// -------------------------------------------------------------
// 2. Lépés: Csak a nem nulla bázisfüggvények kiszámítása
// Ez a "The NURBS Book" A2.2 algoritmusa.
// Csak a releváns p+1 darab súlyt számolja ki.
// -------------------------------------------------------------
void BSplineEvaluateBasisFunctions(int spanIndex, BSplineParams params, out float N[BSPLINE_MAX_DEGREE + 1]) {
    float left[BSPLINE_MAX_DEGREE + 1];
    float right[BSPLINE_MAX_DEGREE + 1];
    
    N[0] = 1.0;

    for (int j = 1; j <= params.degree; ++j) {
        left[j] = params.t - BSplineKnots[spanIndex + 1 - j];
        right[j] = BSplineKnots[spanIndex + j] - params.t;
        
        float saved = 0.0;
        
        for (int r = 0; r < j; ++r) {
            float temp = N[r] / (right[r + 1] + left[j - r]);
            N[r] = saved + right[r + 1] * temp;
            saved = left[j - r] * temp;
        }
        N[j] = saved;
    }
}

// -------------------------------------------------------------
// Optimalizált B-Spline kiértékelés
// -------------------------------------------------------------
vec3 BSpline(BSplineParams params)
{
    // 1. Megkeressük, melyik csomópont-intervallumban vagyunk
    int spanIndex = BSplineFindKnotSpan(params);

    // 2. Kiszámoljuk a p+1 darab nem nulla súlyt
    float weights[BSPLINE_MAX_DEGREE + 1];
    
    // GLSL-ben a tömböket inicializálni kell, vagy biztosítani a méretet.
    // Itt feltételezzük, hogy params.degree <= MAX_DEGREE.
    BSplineEvaluateBasisFunctions(spanIndex, params, weights);

    vec3 C = vec3(0.0);

    // 3. Összegezzük a kontrollpontokat a súlyokkal
    // A spanIndex az a csomópont index, ahol u_i <= t < u_{i+1}.
    // Az ehhez tartozó kontrollpontok az i-p ... i tartományban vannak.
    for (int i = 0; i <= params.degree; ++i) {
        // A kontrollpont indexe: (spanIndex - degree + i)
        int cpIndex = spanIndex - params.degree + i;
        
        // Biztonsági ellenõrzés (opcionális, ha biztosak vagyunk a határokban)
        if(cpIndex >= 0 && cpIndex < params.ctrlPointCount) {
             C += weights[i] * BSplineCtrlPoints[cpIndex].xyz;
        }
    }

    return C;
}