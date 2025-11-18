#pragma once

#include "../include_all.h"

class BSpline : public ModelBase, public IDrawable {
protected:
    std::vector<glm::vec4> m_interpolatedPoints{};
    std::vector<glm::vec4> m_ctrlPoints{};
    std::vector<float> m_knots{};
    GLuint m_interpolatedPointsSSBOID = 0;
    GLuint m_ctrlPointsSSBOID = 0;
    GLuint m_knotsSSBOID = 0;
    bool m_ctrlPointsDirty = false;
    bool m_knotsDirty = false;
    int m_smoothness = 20;
    glm::vec3 m_color = { 1.f, 1.f, 0 };

    static constexpr int m_degree = 3; // cubic only

    void SetCtrlPointsSSBO();
    void WriteCtrlPointsSSBO();
    void SetInterpolatedPointsSSBO();
    void WriteInterpolatedPointsSSBO(std::vector<glm::vec4> points);
    void SetKnotsSSBO();
    void WriteKnotsSSBO();

public:
    /**
     * @brief Creates a cubic B-Spline curve that interpolates the given data points (Q)
     * by automatically calculating the t parameters using the Centripetal Chord Length method.
     * @param dataPoints The points (Q_i) the curve must pass through.
     * @param params BSpline rendering parameters.
     * @return BSpline* The new interpolated B-Spline object, or nullptr on failure.
     */
    static BSpline* CreateInterpolated(
        const std::vector<glm::vec4>& dataPoints,
        BSplineParams params
    );

    /**
     * @brief Creates a cubic B-Spline curve that interpolates the given data points (Q)
     * using user-defined t parameters (t_values).
     * * The t_values MUST have the same size as dataPoints. The t_values define where the
     * curve must pass through Q_i (i.e., Q_i = C(t_i)).
     * * @param dataPoints The points (Q_i) the curve must pass through.
     * @param t_values The user-defined parametric values (t_0, t_1, ..., t_{n-1}).
     * @param params BSpline rendering parameters.
     * @return BSpline* The new interpolated B-Spline object, or nullptr on failure.
     */
    static BSpline* CreateInterpolatedWithParams(
        const std::vector<glm::vec4>& dataPoints,
        const std::vector<float>& t_values,
        BSplineParams params
    );

    BSpline(BSplineParams params);
    ~BSpline();

    void Render(RenderParams* p);
    void RenderSelection(RenderParams* p);
    void RenderInterpolatedPoints(RenderParams* p);

    // Control points
    inline void AddCtrlPoint(glm::vec3 newPoint) {
        m_ctrlPoints.push_back(glm::vec4(newPoint, 1));
        m_ctrlPointsDirty = true;
    }
    inline void DelCtrlPoint(int index) {
        if (index < 0 || index > m_ctrlPoints.size() - 1) {
            Log::errorToConsole("BSpline::DelCtrlPoint invalid index");
            return;
        }
        m_ctrlPoints.erase(m_ctrlPoints.begin() + index);
        m_ctrlPointsDirty = true;
    }
    inline void SetCtrlPoint(int index, glm::vec3 position) {
        if (index < 0 || index > m_ctrlPoints.size() - 1) {
            Log::errorToConsole("BSpline::SetCtrlPoint invalid index");
            return;
        }
        m_ctrlPoints[index] = glm::vec4(position, 1);
        m_ctrlPointsDirty = true;
    }
    inline void SetCtrlPoints(std::vector<glm::vec4> points) {
        m_ctrlPoints = points;
        m_ctrlPointsDirty = true;
    }
    inline std::vector<glm::vec4> GetCtrlPoints() const {
        return m_ctrlPoints;
    }
    inline GLuint GetCtrlPointsSSBO() const {
        return m_ctrlPointsSSBOID;
    }

    // Interpolated points
    inline void SetInterpolatedPoints(std::vector<glm::vec4> points) {
        if (m_interpolatedPoints.size() > 0) {
            Log::errorToConsole("Cannot overwrite interpolated points");
            return;
        }
        SetInterpolatedPointsSSBO();
        WriteInterpolatedPointsSSBO(points);
        m_interpolatedPoints = points;
    }
    inline void ClearInterpolatedPoints() {
        m_interpolatedPoints.erase(m_interpolatedPoints.begin(), m_interpolatedPoints.end());
        glDeleteBuffers(1, &m_interpolatedPointsSSBOID);
        m_interpolatedPointsSSBOID = 0;
    }
    inline std::vector<glm::vec4> GetInterpolatedPoints() {
        return m_interpolatedPoints;
    }
    inline GLuint GetInterpolatedPointsSSBO() const {
        return m_interpolatedPointsSSBOID;
    }
    inline int GetInterpolatedPointsCount() const {
        return m_interpolatedPoints.size();
    }

    // Knot vector
    inline void AddKnot(float knot) {
        m_knots.push_back(knot);
        m_knotsDirty = true;
    }
    inline void DelKnot(int index) {
        if (index < 0 || index >= m_knots.size()) {
            Log::errorToConsole("BSpline::DelKnot invalid index");
            return;
        }
        m_knots.erase(m_knots.begin() + index);
        m_knotsDirty = true;
    }
    inline void SetKnots(std::vector<float> knots) {
        m_knots = knots;
        m_knotsDirty = true;
    }
    inline void SetKnot(int index, float knot) {
        if (index < 0 || index >= m_knots.size()) {
            Log::errorToConsole("Invalid index (BSpline::SetKnot)");
            return;
        }
        m_knots[index] = knot;
        m_knotsDirty = true;
    }
    inline std::vector<float> GetKnots() const {
        return m_knots;
    }
    inline GLuint GetKnotsSSBO() const {
        return m_knotsSSBOID;
    }

    // prevent GetTransform to reset m_transformDirty and Transformation::m_dirty
    inline glm::mat4 GetTransform() const {
        return m_transform;
    }

    // Render settings
    inline void SetSmoothness(int smoothness) {
        m_smoothness = smoothness;
    }
    inline int GetSmoothness() const {
        return m_smoothness;
    }
    inline void SetApplyTransforms(bool apply) {
        m_applyTransforms = apply;
        m_transformDirty = true;
    }
    inline void SetColor(glm::vec3 color) {
        m_color = color;
    }
    inline glm::vec3 GetColor() const {
        return m_color;
    }
    inline int GetDegree() const {
        return m_degree;
    }
    
};