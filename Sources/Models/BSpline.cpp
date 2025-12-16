#include "../../Headers/include_all.h"

BSpline::BSpline(BSplineParams params) : ModelBase(BSPLINE2MODELBASE) {
    m_type = MODEL_TYPE_BSPLINE;
    m_smoothness = params.smoothness;

    // Alap vezérlõpontok
    m_ctrlPoints = {
        glm::vec4{ 10.0, 0.0, 0.0, 1 },
        glm::vec4{ 5.0, 5.0, 1.0, 1 },
        glm::vec4{ 0.0, 10.0, 2.0, 1 },
        glm::vec4{ -5.0, 5.0, 3.0, 1 },
        glm::vec4{ -10.0, 0.0, 4.0, 1 },
        glm::vec4{ -5.0, -5.0, 5.0, 1 },
        glm::vec4{ 0.0, -10.0, 6.0, 1 },
        glm::vec4{ 5.0, -5.0, 7.0, 1 }
    };

    m_knots = { 0,0,0,0,1,1,1,1,2,2,2,2 };
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
	glDeleteBuffers(1, &m_interpolatedPointsSSBOID);
	m_interpolatedPointsSSBOID = 0;
}

void BSpline::SetCtrlPointsSSBO() {
    glGenBuffers(1, &m_ctrlPointsSSBOID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ctrlPointsSSBOID);
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
}
void BSpline::WriteInterpolatedPointsSSBO() {
    if (GetInterpolatedPointsCount() <= 0) {
        return;
    }
    std::vector<glm::vec4> newPoints;
    for (auto& p : m_interpolatedPoints) {
        if (m_applyTransforms)
            newPoints.push_back(GetTransform() * p);
        else
            newPoints.push_back(p);
    }
    m_interpolatedPoints = newPoints;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_interpolatedPointsSSBOID);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
        m_interpolatedPoints.size() * sizeof(glm::vec4),
        m_interpolatedPoints.data(),
        GL_STATIC_DRAW);
}

void BSpline::SetKnotsSSBO() {
    glGenBuffers(1, &m_knotsSSBOID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_knotsSSBOID);
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
    if (!GetShow()) {
        return;
    }

    // -- Check if the curve can be rendered --
    if (m_ctrlPoints.size() < m_degree + 1) {
        Log::errorToConsole("BSpline curve has too few control points");
        SetShow(false);
        return;
    }

    // -- Update ctrlPoints SSBO and transformation matrix if needed --
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
        WriteInterpolatedPointsSSBO();
    }
    if (m_knotsDirty) {
        WriteKnotsSSBO();
    }

    // -- Set render options --
    GLfloat lineWidth;
    glGetFloatv(GL_LINE_WIDTH, &lineWidth);
    glLineWidth(p->lineWidth);

    // -- Activate shader --
    GLuint progID = GetProgramID();
    glUseProgram(progID);

    // -- Set shader input data --
    // BSpline module
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, GetKnotsSSBO());
    glUniform1i(ul(progID, "bSplineData.ctrlPointCount"), GetCtrlPointCount());
    glUniform1i(ul(progID, "bSplineData.knotCount"), GetKnotCount());
    glUniform1i(ul(progID, "bSplineData.degree"), m_degree);
    glUniform1i(ul(progID, "bSplineData.division"), m_smoothness);
    // Camera module
    glUniform3fv(ul(progID, "cameraData.eye"), 1, glm::value_ptr(p->cameraPos));
    glUniformMatrix4fv(ul(progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
    // Color module
    glUniform3fv(ul(progID, "colorData.color"), 1, glm::value_ptr(GetColor()));

    // -- Draw call --
    glDrawArrays(GetDrawMode(), 0, GetSmoothness());

    // -- Restore initial OGL state --
    glLineWidth(lineWidth);
    glUseProgram(0);

    // -- Render selection if needed --
    if (p->selected) {
        RenderSelection(p);
    }

    // -- Render interpolated points if needed --
    if (GetInterpolatedPointsCount() > 0) {
        RenderInterpolatedPoints(p);
    }
}
void BSpline::RenderSelection(RenderParams* p) {
    // -- Activate shader --
    GLuint progID = GetProgramSelectedID();
    glUseProgram(progID);

    // -- Set render options --
    GLfloat pointSize;
    glGetFloatv(GL_POINT_SIZE, &pointSize);
    glPointSize(p->selectionWidth);
    
    // -- Set shader input data --
    // BSpline module
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());
    /* The module defines these uniforms, but the shader doesn't use them
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, GetKnotsSSBO());
    glUniform1i(ul(progID, "bSplineData.ctrlPointCount"), GetCtrlPointCount());
    glUniform1i(ul(progID, "bSplineData.knotCount"), GetKnotCount());
    glUniform1i(ul(progID, "bSplineData.degree"), m_degree);
    glUniform1i(ul(progID, "bSplineData.division"), m_smoothness);
    */
    // Camera module
    glUniform3fv(ul(progID, "cameraData.eye"), 1, glm::value_ptr(p->cameraPos));
    glUniformMatrix4fv(ul(progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
    // Color module
    glUniform3fv(ul(progID, "colorData.color"), 1, glm::value_ptr(p->selectionColor));

    // -- Draw call --
    glDrawArrays(GL_POINTS, 0, GetCtrlPoints().size());

    // -- Restore initial OGL state --
    glPointSize(pointSize);
    glUseProgram(0);
}
void BSpline::RenderGUI(std::vector<ModelBase*>* models) {
    ImGui::Text("B-Spline specific options");
    ImGui::Spacing();

    BSpline* b = this;

    // Smoothness
    int smoothness = b->GetSmoothness();
    if (ImGui::SliderInt("Smoothness", &smoothness, 2, 64)) {
        b->SetSmoothness(smoothness);
    }

    // ctrl points
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Control points")) {
        int ctrlPointCount = 0;
        for (auto p : b->GetCtrlPoints()) {
            glm::vec3 point = p;
            std::stringstream label;
            label << "Ctrl point " << ctrlPointCount;
            if (ImGui::InputFloat3(label.str().c_str(), &point.x)) {
                b->SetCtrlPoint(ctrlPointCount, point);
            }
            ImGui::SameLine();
            label.str("");
            label << "Delete #" << ctrlPointCount;
            if (ImGui::Button(label.str().c_str())) {
                b->DelCtrlPoint(ctrlPointCount);
            }
            ++ctrlPointCount;
        }
        ImGui::Spacing();
        ImGui::InputFloat3("New ctrl point", &m_bsplineNewCtrlPoint.x);
        ImGui::SameLine();
        if (ImGui::Button("Add")) {
            b->AddCtrlPoint(m_bsplineNewCtrlPoint);
        }
    }

    // knot vector
    if (ImGui::CollapsingHeader("Knot vector")) {
        int knotCount = 0;
        for (auto k : b->GetKnots()) {
            float knot = k;
            std::stringstream label;
            label << "Knot #" << knotCount;
            if (ImGui::InputFloat(label.str().c_str(), &knot)) {
                b->SetKnot(knotCount, knot);
            }
            ImGui::SameLine();
            label.str("");
            label << "Delete #" << knotCount;
            if (ImGui::Button(label.str().c_str())) {
                b->DelKnot(knotCount);
            }
            ++knotCount;
        }
        ImGui::Spacing();
        ImGui::InputFloat("New knot", &m_newKnot);
        ImGui::SameLine();
        if (ImGui::Button("Add")) {
            b->AddKnot(m_newKnot);
        }
    }

    // interpolation points
    if (ImGui::CollapsingHeader("interpolated points")) {
        int intPointCount = 0;
        for (auto p : b->GetInterpolatedPoints()) {
            glm::vec3 point = p;
            std::stringstream label;
            label << "Interpolated point " << intPointCount;
            ImGui::InputFloat3(label.str().c_str(), &point.x);
            ++intPointCount;
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // color
    glm::vec3 col = b->GetColor();
    m_curveColor[0] = col.r;
    m_curveColor[1] = col.g;
    m_curveColor[2] = col.b;
    if (ImGui::ColorEdit3("Color", &m_curveColor.r)) {
        b->SetColor(m_curveColor);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void BSpline::RenderInterpolatedPoints(RenderParams* p) {
    // -- Activate shader --
    GLuint progID = GetProgramSelectedID();
    glUseProgram(progID);

    // -- Set render options --
    GLfloat pointSize;
    glGetFloatv(GL_POINT_SIZE, &pointSize);
    glPointSize(p->selectionWidth);

    // -- Set shader input data --
    // BSpline module
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetInterpolatedPointsSSBO());
    /* The module defines these uniforms, but the shader doesn't use them
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, GetKnotsSSBO());
    glUniform1i(ul(progID, "bSplineData.ctrlPointCount"), GetCtrlPointCount());
    glUniform1i(ul(progID, "bSplineData.knotCount"), GetKnotCount());
    glUniform1i(ul(progID, "bSplineData.degree"), m_degree);
    glUniform1i(ul(progID, "bSplineData.division"), m_smoothness);
    */
    // Camera module
    glUniform3fv(ul(progID, "cameraData.eye"), 1, glm::value_ptr(p->cameraPos));
    glUniformMatrix4fv(ul(progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
    // Color module
    glUniform3fv(ul(progID, "colorData.color"), 1, glm::value_ptr(GetColor()));

    // -- Draw call --
    glDrawArrays(GL_POINTS, 0, GetInterpolatedPointsCount());

    // reset gl state
    glPointSize(pointSize);
    glUseProgram(0);
}
