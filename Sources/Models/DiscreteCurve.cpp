#include "../../Headers/include_all.h"

DiscreteCurve::DiscreteCurve(DiscreteCurveParams params) : ModelBase(DISCRETECURVE2MODELBASE) {
	m_type = MODEL_TYPE_DISCRETECURVE;
	m_ctrlPoints = {
		glm::vec4{0,0,0,1},
		glm::vec4{1,0,0,1},
		glm::vec4{0,1,0,1},
	};
	SetCtrlPointsSSBO();
}
DiscreteCurve::~DiscreteCurve() {
	glDeleteBuffers(1, &m_ctrlPointsSSBOID);
}

void DiscreteCurve::Render(RenderParams* p) {
	if (!GetShow()) {
		return;
	}

	// -- Check if the surface can be rendered --
	if (GetCtrlPoints().size() < 2) {
		Log::errorToConsole("DiscreteCurve \"", GetName().c_str(), "\" has too few control points");
		SetShow(false);
		return;
	}

	// -- Update ctrlPoints SSBO and transformation matrix if needed --
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

	// -- Set render options --
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(p->lineWidth);

	// -- Activate shader --
	GLuint progID = GetProgramID();
	glUseProgram(progID);

	// -- Set shader input data --
	// Discrete curve module
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());
	// Camera module
	glUniform3fv(ul(progID, "cameraData.cameraPos"), 1, glm::value_ptr(p->cameraPos));
	glUniformMatrix4fv(ul(progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	// Color module
	glUniform3fv(ul(progID, "colorData.color"), 1, glm::value_ptr(GetColor()));

	// -- Draw call --
	glDrawArrays(m_drawMode, 0, GetCtrlPointCount());

	// -- Restore initial OGL state --
	glLineWidth(lineWidth);
	glUseProgram(0);

	// -- Render selection if needed --
	if (p->selected) {
		RenderSelection(p);
	}
}
void DiscreteCurve::RenderSelection(RenderParams* p) {
	// -- Activate shader --
	GLuint progID = GetProgramSelectedID();
	glUseProgram(progID);

	// -- Set render options --
	GLfloat pointSize;
	glGetFloatv(GL_POINT_SIZE, &pointSize);
	glPointSize(p->selectionWidth);

	// -- Set shader input data --
	// Discrete curve module
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());
	// Camera module
	glUniform3fv(ul(progID, "cameraData.cameraPos"), 1, glm::value_ptr(p->cameraPos));
	glUniformMatrix4fv(ul(progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	// Color module
	glUniform3fv(ul(progID, "colorData.color"), 1, glm::value_ptr(p->selectionColor));

	// -- Draw call --
	glDrawArrays(GL_POINTS, 0, GetCtrlPointCount());

	// -- Restore initial OGL state --
	glPointSize(pointSize);
	glUseProgram(0);
}
void DiscreteCurve::RenderGUI(std::vector<ModelBase*>* models) {
	ImGui::Text("Discrete-curve specific options");

	DiscreteCurve* d = this;

	// ctrl points
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Control points")) {
		int ctrlPointCount = 0;
		for (auto p : d->GetCtrlPoints()) {
			glm::vec3 point = p;
			std::stringstream label;
			label << "Ctrl point " << ctrlPointCount;
			if (ImGui::InputFloat3(label.str().c_str(), &point.x)) {
				d->SetCtrlPoint(ctrlPointCount, point);
				break;
			}
			ImGui::SameLine();
			label.str("");
			label << "Delete #" << ctrlPointCount;
			if (ImGui::Button(label.str().c_str())) {
				d->DelCtrlPoint(ctrlPointCount);
				break;
			}
			++ctrlPointCount;
		}
		ImGui::Spacing();
		ImGui::InputFloat3("New ctrl point", &m_discreteCurveNewCtrlPoint.x);
		ImGui::SameLine();
		if (ImGui::Button("Add")) {
			d->AddCtrlPoint(m_discreteCurveNewCtrlPoint);
		}
	}

	ImGui::Spacing();
	ImGui::Spacing();

	// color
	glm::vec3 col = d->GetColor();
	m_curveColor[0] = col.r;
	m_curveColor[1] = col.g;
	m_curveColor[2] = col.b;
	if (ImGui::ColorEdit3("Color", &m_curveColor.r)) {
		d->SetColor(m_curveColor);
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
}