#include "../../Headers/include_all.h"

BezierSurface::BezierSurface(BezierSurfaceParams params) : ModelBase(BEZIERSURFACE2MODELBASE) {
	m_wireframe = params.wireframe;
	m_type = MODEL_TYPE_BEZIERSURFACE;
	m_smoothness = params.smoothness;
	SetCtrlPointsSSBO();
}
BezierSurface::~BezierSurface() {
	glDeleteBuffers(1, &m_ctrlPointsSSBOID);
	m_ctrlPointsSSBOID = 0;

	if (m_material != nullptr) {
		delete(m_material);
	}
}

void BezierSurface::Render(RenderParams* p) {
	if (!GetShow()) {
		return;
	}

	if (GetCtrlPoints().size() < 1) {
		Log::errorToConsole("Bezier-surface \"", GetName().c_str(), "\" has too few control points");
		SetShow(false);
		return;
	}
	if (GetCtrlPoints().size() != GetRowsCount() * GetColsCount()) {
		Log::errorToConsole("Bezier-surface \"", GetName().c_str(), "\" dimensions do not match");
		SetShow(false);
		return;
	}
	if (GetMaterial() == nullptr) {
		Log::errorToConsole("Corrupted material found");
		exit(1);
	}

	bool transformsReset = false;
	// check if any of the transformations is changed
	bool isDirty = false;
	for (auto t : m_transforms) {
		if (t->IsDirty()) {
			isDirty = true;
		}
		t->Clean();
	}
	// calculate transformation and set SSBO if changed
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

	bool cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
	GLfloat defLineWidth;
	glGetFloatv(GL_LINE_WIDTH, &defLineWidth);
	if (GetWireFrame()) {
		glDisable(GL_CULL_FACE);
		glLineWidth(p->lineWidth);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	GLuint progID = GetProgramID();
	glUseProgram(progID);

	// set SSBO
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());

	// set uniforms
	glUniform3fv(ul(progID, "cameraPos"), 1, glm::value_ptr(p->cameraPos));
	glUniform1i(ul(progID, "modelID"), p->modelIndex);
	glUniform2fv(ul(progID, "cursorPos"), 1, glm::value_ptr(p->cursorPos));
	glUniform2fv(ul(progID, "windowSize"), 1, glm::value_ptr(p->windowSize));
	glUniformMatrix4fv(ul(progID, "viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	glUniform2iv(ul(progID, "ctrlPointCount"), 1, glm::value_ptr(GetDimensions()));
	glUniform2iv(ul(progID, "division"), 1, glm::value_ptr(GetSmoothness()));

	// set material
	Material::UploadMaterialToShader(progID, GetMaterial());

	// set light
	Light light = Light();
	if (p->lights.size() > 0) {
		light.pos = p->lights[0];
	}
	Light::UploadLightToShader(progID, &light);

	glDrawArrays(GL_TRIANGLES, 0, (GetSmoothness().x - 1) * (GetSmoothness().y - 1) * 2 * 3);

	// restore initial OGL state
	if (cullFaceEnabled) glEnable(GL_CULL_FACE);
	glLineWidth(defLineWidth);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUseProgram(0);

	if (p->selected) {
		RenderSelection(p);
	}
}
void BezierSurface::RenderSelection(RenderParams* p) {
	GLuint progID = GetProgramSelectedID();
	glUseProgram(progID);

	// set SSBO
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());

	// set uniforms
	glUniform2iv(ul(progID, "ctrlPointCount"), 1, glm::value_ptr(GetDimensions()));
	glUniformMatrix4fv(ul(progID, "viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	glUniform3fv(ul(progID, "selColor"), 1, glm::value_ptr(p->selectionColor));

	// set point size
	GLfloat pointSize;
	glGetFloatv(GL_POINT_SIZE, &pointSize);
	glPointSize(p->selectionWidth);

	// draw control points
	glDrawArrays(GL_POINTS, 0, GetCtrlPoints().size());

	// reset gl state
	glPointSize(pointSize);

	return;
}
void BezierSurface::RenderGUI(std::vector<ModelBase*>* models) {
	ImGui::Text("Bezier-curve specific options");
	ImGui::Spacing();

	BezierSurface* b = this;

	// Smoothness
	int smoothness[2]{ b->GetSmoothness().x, b->GetSmoothness().y };
	if (ImGui::SliderInt2("Smoothness", smoothness, 2, 16)) {
		b->SetSmoothness(glm::vec2(smoothness[0], smoothness[1]));
	}

	// Wireframe
	bool wireframe = b->GetWireFrame();
	if (ImGui::Checkbox("Wireframe", &wireframe)) {
		b->SetWireFrame(wireframe);
	}

	// ctrl points
	/*
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
		glm::vec3 point = m_bezierNewCtrlPoint;
		if (ImGui::InputFloat3("New ctrl point", &point.x)) {
			m_bezierNewCtrlPoint = point;
		}
		ImGui::SameLine();
		if (ImGui::Button("Add")) {
			b->AddCtrlPoint(m_bezierNewCtrlPoint);
		}
	}
	ImGui::Spacing();
	*/

	ImGui::Separator();
	ImGui::Spacing();
}