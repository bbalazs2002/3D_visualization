#pragma once

#include "../include_all.h"

namespace BSplineInterpolation {

    constexpr float EPSILON = 1e-5f;

    // ----------------------------------------------------------------------
    // GLM VEC4 SEGÉDFÜGGVÉNYEK
    // ----------------------------------------------------------------------
    inline glm::vec4 DivideVec4ByScalar(const glm::vec4& vec, float scalar) {
        if (std::abs(scalar) < EPSILON) {
            // A Thomas algoritmusban a kritikus hibákat kezelni kell, de itt csak logolunk a debugoláshoz.
            // Valós alkalmazásban ennek jeleznie kell a mátrix szingularitását.
            // Log::errorToConsole("Error: Division by near-zero scalar in SolveInterpolation!");
            return vec * 100000.0f;
        }
        return vec / scalar;
    }


    // ----------------------------------------------------------------------
    // 1. COX-DE BOOR FÜGGVÉNY (N_{i,p}(t)) - ROBUST VERZIÓ
    // ----------------------------------------------------------------------

    /**
     * @brief A robusztus Cox-de Boor algoritmus a B-Spline alapfüggvényekhez.
     * Ez a verzió expliciten kezeli a $0/0$ eseteket (amelyek a Clamped végpontoknál jelentkeznek)
     * a $p>0$ rekurzióban.
     */
    inline float CoxDeBoor(int i, int p, float t, const std::vector<float>& U) {
        // Alap eset: p=0 (szakaszos konstans függvény)
        if (p == 0) {
            // A B-Spline definíció szerint az N_i,0(t) = 1, ha U_i <= t < U_{i+1}.
            // Mivel Clamped spline-t használunk, a t=1.0 végpontot is tartalmaznia kell az utolsó N-nek.
            if (U[i] <= t && t < U[i + 1]) {
                return 1.0f;
            }
            // Speciális kezelés a t=1.0 végpontra (Csak akkor hívódik, ha t=1.0)
            // t=1.0 esetén N_{N-1,p}(1.0)=1.0, a többi N_{i,p}(1.0)=0.0.
            // A rekurzió során N_{N-1,p}(1.0) a p=0 szinten az U.size()-2 indexû intervallumot éri el.
            if (i == U.size() - 2 && std::abs(t - 1.0f) < EPSILON) {
                return 1.0f;
            }
            return 0.0f;
        }

        // --- Bal Oldali Tag: (t - U[i]) / (U[i + p] - U[i]) * N_{i, p-1}(t) ---
        float leftTerm = 0.0f;
        float denomLeft = U[i + p] - U[i];

        // Elkerüljük a 0/0 esetet: Ha a nevezõ 0, a tag értéke 0.
        if (denomLeft > EPSILON) {
            float numLeft = t - U[i];
            leftTerm = (numLeft / denomLeft) * CoxDeBoor(i, p - 1, t, U);
        }

        // --- Jobb Oldali Tag: (U[i + p + 1] - t) / (U[i + p + 1] - U[i + 1]) * N_{i + 1, p-1}(t) ---
        float rightTerm = 0.0f;
        float denomRight = U[i + p + 1] - U[i + 1];

        // Elkerüljük a 0/0 esetet: Ha a nevezõ 0, a tag értéke 0.
        if (denomRight > EPSILON) {
            float numRight = U[i + p + 1] - t;
            rightTerm = (numRight / denomRight) * CoxDeBoor(i + 1, p - 1, t, U);
        }

        return leftTerm + rightTerm;
    }


    // ----------------------------------------------------------------------
    // 2. CSOMÓVEKTOR GENERÁLÁSA (U)
    // ----------------------------------------------------------------------

    inline std::vector<float> GenerateKnotVector(int N, int p, const std::vector<float>& t_values) {
        int M_plus_1 = N + p + 1;
        std::vector<float> U(M_plus_1);

        // 1. Elsõ p+1 csomó 0.0 (Clamped)
        for (int i = 0; i <= p; ++i) {
            U[i] = 0.0f;
        }

        // 2. Belsõ csomók: U_{j} = t_{j-p}
        // U index: p+1-tõl N-1-ig. (t index: 1-tõl N-p-1-ig)
        for (int j = p + 1; j < N; ++j) {
            // Ezzel U_{p+1} = t_1, ..., U_{N-1} = t_{N-p-1}
            U[j] = t_values[j - p];
        }

        // 3. Utolsó p+1 csomó 1.0 (Clamped)
        for (int i = N; i < M_plus_1; ++i) {
            U[i] = 1.0f;
        }

        return U;
    }


    // ----------------------------------------------------------------------
    // 3. THOMAS ALGORITMUS (TRIDIAGONÁLIS MÁTRIX MEGOLDÁSA)
    // ----------------------------------------------------------------------

    inline std::vector<glm::vec4> SolveInterpolation(
        const std::vector<float>& a,
        const std::vector<float>& b,
        const std::vector<float>& c,
        const std::vector<glm::vec4>& D,
        int N
    ) {
        if (N <= 0) return {};

        std::vector<float> c_prime(N);
        std::vector<glm::vec4> D_prime(N);
        std::vector<glm::vec4> P(N);

        // 1. Elõrehaladó kiiktatás (Forward Elimination)
        c_prime[0] = c[0] / b[0];
        D_prime[0] = DivideVec4ByScalar(D[0], b[0]);

        for (int i = 1; i < N; ++i) {
            float m = b[i] - a[i] * c_prime[i - 1];

            // Kritikus ellenõrzés a numerikus stabilitásért
            if (std::abs(m) < EPSILON) {
                Log::errorToConsole("Interpolation failure: Thomas algorithm diagonal element 'm' is near zero. Matrix is singular.");
                // Itt valószínûleg a CoxDeBoor függvény által visszaadott b[i] volt túl kicsi.
                return {};
            }

            if (i < N - 1) {
                c_prime[i] = c[i] / m;
            }
            D_prime[i] = DivideVec4ByScalar(D[i] - D_prime[i - 1] * a[i], m);
        }

        // 2. Visszahelyettesítés (Back Substitution)
        P[N - 1] = D_prime[N - 1];

        for (int i = N - 2; i >= 0; --i) {
            P[i] = D_prime[i] - P[i + 1] * c_prime[i];
        }

        return P;
    }


    // ----------------------------------------------------------------------
    // 4. CORE INTERPOLÁCIÓS LOGIKA ((N-2)x(N-2) RENDSZER)
    // ----------------------------------------------------------------------

    inline BSpline* CoreInterpolation(
        int N,
        const std::vector<glm::vec4>& dataPoints,
        const std::vector<float>& t_values,
        int p,
        BSplineParams params
    ) {
        if (N < p + 1) {
            Log::errorToConsole("Interpolation error: Need at least ", p + 1, " data points for degree ", p, " B-Spline.");
            return nullptr;
        }

        int N_sys = N - 2;

        if (N_sys <= 0) {
            Log::errorToConsole("Interpolation error: System size is too small (N-2 <= 0).");
            return nullptr;
        }

        // --- STEP 1: Generate Knot Vector (U) ---
        std::vector<float> U = GenerateKnotVector(N, p, t_values);

        // --- STEP 2: Setup Linear System (A * P_{internal} = D_{modified}) ---

        std::vector<float> a(N_sys);
        std::vector<float> b(N_sys);
        std::vector<float> c(N_sys);
        std::vector<glm::vec4> D_modified(N_sys);

        const glm::vec4& Q0 = dataPoints.front();
        const glm::vec4& QN_1 = dataPoints.back();

        // Ciklus az interpolálandó adatpontokon: Q_1-tõl Q_{N-2}-ig.
        for (int i = 1; i <= N - 2; ++i) {
            int j = i - 1; // Mátrix sor indexe: 0-tól N_sys-1-ig.
            float t_i = t_values[i]; // Paraméterérték Q_i-hez.

            // a[j] (Sub-diagonal): N_{i-1, p}(t_i)
            a[j] = CoxDeBoor(i - 1, p, t_i, U);
            if (j == 0) a[j] = 0.0f;

            // b[j] (Main-diagonal): N_{i, p}(t_i)
            b[j] = CoxDeBoor(i, p, t_i, U);

            // Ha a fõátló elem túl kicsi, ez szingularitásra utal!
            if (b[j] < EPSILON) {
                Log::errorToConsole("CRITICAL ERROR: Main diagonal element b[", j, "] (N_{", i, ",", p, "}(t_", i, ")) is near zero (", b[j], "). Matrix is ill-conditioned.");
            }

            // c[j] (Super-diagonal): N_{i+1, p}(t_i)
            c[j] = CoxDeBoor(i + 1, p, t_i, U);
            if (j == N_sys - 1) c[j] = 0.0f;

            // --- Jobb Oldal (D) Módosítás ---
            glm::vec4 Di = dataPoints[i]; // Q_i

            // Elsõ egyenlet (i=1): levonjuk P_0 = Q_0 hatását
            if (i == 1) {
                // Di = Q_1 - N_{0,p}(t_1) * P_0
                Di -= CoxDeBoor(0, p, t_i, U) * Q0;
            }

            // Utolsó egyenlet (i=N-2): levonjuk P_{N-1} = Q_{N-1} hatását
            if (i == N - 2) {
                // Di = Q_{N-2} - N_{N-1,p}(t_{N-2}) * P_{N-1}
                Di -= CoxDeBoor(N - 1, p, t_i, U) * QN_1;
            }

            D_modified[j] = Di;
        }

        // --- STEP 3: Solve the System (Thomas Algorithm) ---
        std::vector<glm::vec4> P_internal = SolveInterpolation(a, b, c, D_modified, N_sys);

        if (P_internal.empty()) {
            return nullptr;
        }

        // --- STEP 4: Create Final BSpline Control Point Vector ---
        std::vector<glm::vec4> P_ctrlPoints;
        P_ctrlPoints.push_back(Q0); // P_0 = Q_0
        P_ctrlPoints.insert(P_ctrlPoints.end(), P_internal.begin(), P_internal.end()); // P_1, ..., P_{N-2}
        P_ctrlPoints.push_back(QN_1); // P_{N-1} = Q_{N-1}

        if (P_ctrlPoints.size() != N) {
            Log::errorToConsole("Interpolation error: Final control point count mismatch.");
            return nullptr;
        }

        // --- STEP 5: Create BSpline Object ---
        BSpline* newSpline = new BSpline(params);
        newSpline->SetCtrlPoints(P_ctrlPoints);
        newSpline->SetInterpolatedPoints(dataPoints);
        newSpline->SetKnots(U);

        return newSpline;
    }


    // ----------------------------------------------------------------------
    // 5. PARAMÉTEREZÉS (STABIL CENTRIPETÁLIS)
    // ----------------------------------------------------------------------
    // A spirál görbe miatt a centripetális módszer itt elengedhetetlen a stabilitáshoz.

    inline std::vector<float> CalculateParametricTs(const std::vector<glm::vec4>& dataPoints, float ALPHA) {
        int N = dataPoints.size();
        if (N == 0) return {};
        if (N == 1) return { 0.0f };

        std::vector<float> t_values(N);
        t_values[0] = 0.0f;

        float totalLength = 0.0f;
        std::vector<float> chordLengths(N - 1);

        // 1. Távolságok számítása (3D)
        for (int i = 0; i < N - 1; ++i) {
            float dx = dataPoints[i + 1].x - dataPoints[i].x;
            float dy = dataPoints[i + 1].y - dataPoints[i].y;
            float dz = dataPoints[i + 1].z - dataPoints[i].z;

            float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
            float distance_alpha = std::pow(distance, ALPHA);
            chordLengths[i] = distance_alpha;
            totalLength += distance_alpha;
        }

        // 2. t_értékek normalizálása
        if (totalLength < EPSILON) {
            for (int i = 1; i < N; ++i) {
                t_values[i] = (float)i / (N - 1.0f);
            }
        }
        else {
            for (int i = 1; i < N; ++i) {
                t_values[i] = t_values[i - 1] + chordLengths[i - 1];
            }
            // Végleges normalizálás [0, 1] tartományba
            for (float& t : t_values) {
                t /= totalLength;
            }
        }

        return t_values;
    }

} // namespace BSplineInterpolation

// ----------------------------------------------------------------------
// BSPLINE OSZTÁLY METÓDUSAI
// ----------------------------------------------------------------------

// Implementation of the static method in the BSpline class
inline BSpline* BSpline::CreateInterpolatedWithParams(
    const std::vector<glm::vec4>& dataPoints,
    const std::vector<float>& t_values_raw, // t_values_raw névvel, a tisztánlátás kedvéért
    BSplineParams params
) {
    int N = dataPoints.size();
    int p = 3; // Cubic B-Spline

    if (N != t_values_raw.size()) {
        Log::errorToConsole("Error in CreateInterpolatedWithParams: dataPoints and t_values must have the same size.");
        return nullptr;
    }
    if (N < p + 1) {
        Log::errorToConsole("Error in CreateInterpolatedWithParams: Need at least ", p + 1, " points for cubic B-Spline.");
        return nullptr;
    }

    // --- KRITIKUS JAVÍTÁS: T Paraméterek Normalizálása ---
    std::vector<float> normalized_t_values = t_values_raw;

    if (!t_values_raw.empty()) {
        float t_min = t_values_raw.front();
        float t_max = t_values_raw.back();
        float t_range = t_max - t_min;

        if (t_range < BSplineInterpolation::EPSILON) {
            Log::errorToConsole("Interpolation error: t_values must span a non-zero range.");
            return nullptr;
        }

        // Normalizálás a 0.0 - 1.0 tartományba
        for (float& t : normalized_t_values) {
            t = (t - t_min) / t_range;
        }
    }
    // Ezzel biztosítjuk, hogy a CoreInterpolation mindig stabil, 0..1 tartományban lévõ t értékekkel dolgozzon.

    // Átadjuk a normalizált t_values-t
    return BSplineInterpolation::CoreInterpolation(N, dataPoints, normalized_t_values, p, params);
}

// Implementation of the static method in the BSpline class
inline BSpline* BSpline::CreateInterpolated(
    const std::vector<glm::vec4>& dataPoints,
    BSplineParams params
) {
    // 1. Calculate parameters (t_i) using the Centripetal Chord Length method
    std::vector<float> t_values = BSplineInterpolation::CalculateParametricTs(dataPoints, 0.5f);

    return CreateInterpolatedWithParams(dataPoints, t_values, params);
}