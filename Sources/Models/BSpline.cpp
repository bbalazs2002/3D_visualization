#include "../../Headers/include_all.h"

BSpline::BSpline(BSplineParams params) : ModelBase(BSPLINE2MODELBASE) {
    m_type = MODEL_TYPE_BSPLINE;
    m_smoothness = params.smoothness;

    // Alap vezérlõpontok
    m_ctrlPoints = {
        {0, 0, 0, 1},
        {1, 0, 0, 1},
        {2, 1, 0, 1},
        {3, 1, 0, 1},
        {4, 0, 0, 1}
    };

    m_knots = params.knots;
    m_smoothness = params.smoothness;

    SetCtrlPointsSSBO();
    SetKnotsSSBO();
}

BSpline::~BSpline() {
    ClearInterpolatedPoints();
    glDeleteBuffers(1, &m_ctrlPointsSSBOID);
    m_ctrlPointsSSBOID = 0;
    glDeleteBuffers(1, &m_knotsSSBOID);
    m_knotsSSBOID = 0;
}

void BSpline::SetCtrlPointsSSBO() {
    glGenBuffers(1, &m_ctrlPointsSSBOID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ctrlPointsSSBOID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ctrlPointsSSBOID);
    WriteCtrlPointsSSBO();
}

void BSpline::WriteCtrlPointsSSBO() {
    m_ctrlPointsDirty = false;
    std::vector<glm::vec4> newPoints;
    for (auto& p : m_ctrlPoints) {
        if (m_applyTransforms)
            newPoints.push_back(GetTransform() * p);
        else
            newPoints.push_back(p);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ctrlPointsSSBOID);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
        newPoints.size() * sizeof(glm::vec4),
        newPoints.data(),
        GL_STATIC_DRAW);
}

void BSpline::SetInterpolatedPointsSSBO() {
    glGenBuffers(1, &m_interpolatedPointsSSBOID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_interpolatedPointsSSBOID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_interpolatedPointsSSBOID);
}

void BSpline::WriteInterpolatedPointsSSBO(std::vector<glm::vec4> points) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_interpolatedPointsSSBOID);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
        points.size() * sizeof(glm::vec4),
        points.data(),
        GL_STATIC_DRAW);
}

void BSpline::SetKnotsSSBO() {
    glGenBuffers(1, &m_knotsSSBOID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_knotsSSBOID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_knotsSSBOID);
    WriteKnotsSSBO();
}

void BSpline::WriteKnotsSSBO() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_knotsSSBOID);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
        m_knots.size() * sizeof(float),
        m_knots.data(),
        GL_STATIC_DRAW);
}

void BSpline::Render(RenderParams* p) {
    if (!GetShow()) return;
    if (m_ctrlPoints.size() < m_degree + 1) {
        Log::errorToConsole("BSpline curve has too few control points");
        SetShow(false);
        return;
    }

    bool transformsReset = false;
    bool isDirty = false;
    for (auto t : m_transforms) {
        if (t->IsDirty()) {
            isDirty = true;
            t->Clean();
        }
    }

    if (isDirty || m_transformDirty) {
        m_transformDirty = false;
        glm::mat4 acc = glm::identity<glm::mat4>();
        for (int i = m_transforms.size() - 1; i >= 0; --i) {
            acc *= m_transforms[i]->Get();
        }
        m_transform = acc;
        transformsReset = true;
    }

    if (transformsReset || m_ctrlPointsDirty) {
        WriteCtrlPointsSSBO();
    }
    if (m_knotsDirty) {
        WriteKnotsSSBO();
    }

    GLuint progID = GetProgramID();
    glUseProgram(progID);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ctrlPointsSSBOID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_knotsSSBOID);

    glUniform1i(ul(progID, "ctrlPointCount"), (int)m_ctrlPoints.size());
    glUniform1i(ul(progID, "degree"), m_degree);
    glUniform1i(ul(progID, "division"), m_smoothness);
    glUniform1i(ul(progID, "knotCount"), (int)m_knots.size());
    glUniformMatrix4fv(ul(progID, "viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
    glUniform3fv(ul(progID, "color"), 1, glm::value_ptr(m_color));

    GLfloat lineWidth;
    glGetFloatv(GL_LINE_WIDTH, &lineWidth);
    glLineWidth(p->lineWidth);

    glDrawArrays(m_drawMode, 0, 1);
    glLineWidth(lineWidth);

    if (p->selected) {
        RenderSelection(p);
    }
    if (GetInterpolatedPointsCount() > 0) {
        RenderInterpolatedPoints(p);
    }
}

void BSpline::RenderSelection(RenderParams* p) {
    GLuint progID = GetProgramSelectedID();

    glUseProgram(progID);

    // set SSBO
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());

    // set uniforms
    glUniform1i(ul(progID, "ctrlPointCount"), GetCtrlPoints().size());
    glUniformMatrix4fv(ul(progID, "viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
    glUniform3fv(ul(progID, "selColor"), 1, glm::value_ptr(p->selectionColor));
    glUniform1i(ul(progID, "isSelection"), 1);

    // set point size
    GLfloat pointSize;
    glGetFloatv(GL_POINT_SIZE, &pointSize);
    glPointSize(p->selectionWidth);

    // draw control points
    glDrawArrays(m_drawMode, 0, 1);

    // reset gl state
    glPointSize(pointSize);

    return;
}

void BSpline::RenderInterpolatedPoints(RenderParams* p) {
    GLuint progID = GetProgramSelectedID();
    glUseProgram(progID);

    // set SSBO
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetInterpolatedPointsSSBO());

    // set uniforms
    glUniform1i(ul(progID, "ctrlPointCount"), GetInterpolatedPointsCount());
    glUniformMatrix4fv(ul(progID, "viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
    glUniform3fv(ul(progID, "selColor"), 1, glm::value_ptr(GetColor()));
    glUniform1i(ul(progID, "isSelection"), 1);

    // set point size
    GLfloat pointSize;
    glGetFloatv(GL_POINT_SIZE, &pointSize);
    glPointSize(p->selectionWidth);

    // draw control points
    glDrawArrays(GL_POINTS, 0, 1);

    // reset gl state
    glPointSize(pointSize);

    return;
}
