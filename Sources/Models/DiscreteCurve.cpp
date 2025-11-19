#include "../../Headers/include_all.h"

DiscreteCurve::DiscreteCurve(DiscreteCurveParams params) : ModelBase(BEZIER2MODELBASE) {
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

	if (GetCtrlPoints().size() < 2) {
		Log::errorToConsole("DiscreteCurve \"", GetName().c_str(), "\" has too few control points");
		SetShow(false);
		return;
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

	GLuint progID = GetProgramID();
	glUseProgram(progID);

	// set SSBO
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());

	// set uniforms
	glUniformMatrix4fv(ul(progID, "viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	glUniform3fv(ul(progID, "color"), 1, glm::value_ptr(GetColor()));

	// set line width
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(p->lineWidth);

	glDrawArrays(m_drawMode, 0, GetCtrlPoints().size());

	// reset gl state
	glLineWidth(lineWidth);

	if (p->selected) {
		RenderSelection(p);
	}
}
void DiscreteCurve::RenderSelection(RenderParams* p) {
	GLuint progID = GetProgramSelectedID();
	glUseProgram(progID);

	// set SSBO
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());

	// set uniforms
	glUniformMatrix4fv(ul(progID, "viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	glUniform3fv(ul(progID, "selColor"), 1, glm::value_ptr(p->selectionColor));
	glUniform1i(ul(progID, "isSelection"), 1);

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