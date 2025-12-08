#include "../../Headers/include_all.h"

BezierCurve::BezierCurve(BezierCurveParams params) : ModelBase(BEZIERCURVE2MODELBASE) {
	m_type = MODEL_TYPE_BEZIER;
	m_smoothness = params.smoothness;
	m_ctrlPoints = {
		glm::vec4{0,0,0,1},
		glm::vec4{1,0,0,1},
		glm::vec4{0,1,0,1},
	};
	SetCtrlPointsSSBO();
}
BezierCurve::~BezierCurve() {
	glDeleteBuffers(1, &m_ctrlPointsSSBOID);
}

void BezierCurve::Render(RenderParams* p) {
	if (!GetShow()) {
		return;
	}

	// -- Check if the curve can be rendered --
	if (GetCtrlPoints().size() < 2) {
		Log::errorToConsole("Bezier-curve \"", GetName().c_str(), "\" has too few control points");
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
	// Bezier curve module
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());
	glUniform1i(ul(progID, "bezierCurveData.ctrlPointCount"), GetCtrlPointCount());
	glUniform1i(ul(progID, "bezierCurveData.division"), GetSmoothness());
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
}
void BezierCurve::RenderSelection(RenderParams* p) {
	// -- Activate shader --
	GLuint progID = GetProgramSelectedID();
	glUseProgram(progID);

	// -- Set render options --
	GLfloat pointSize;
	glGetFloatv(GL_POINT_SIZE, &pointSize);
	glPointSize(p->selectionWidth);

	// -- Set shader input data --
	// Bezier curve module
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetCtrlPointsSSBO());
	glUniform1i(ul(progID, "bezierCurveData.ctrlPointCount"), GetCtrlPointCount());
	glUniform1i(ul(progID, "bezierCurveData.division"), GetSmoothness());
	// Camera module
	glUniform3fv(ul(progID, "cameraData.eye"), 1, glm::value_ptr(p->cameraPos));
	glUniformMatrix4fv(ul(progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	// Color module
	glUniform3fv(ul(progID, "colorData.color"), 1, glm::value_ptr(p->selectionColor));

	// -- Draw call --
	glDrawArrays(GL_POINTS, 0, GetCtrlPointCount());

	// -- Restore initial OGL state --
	glPointSize(pointSize);
	glUseProgram(0);

	return;
}
void BezierCurve::RenderGUI(std::vector<ModelBase*>* models) {
	ImGui::Text("Bezier-curve specific options");
	ImGui::Spacing();

	BezierCurve* b = this;

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

	// color
	glm::vec3 col = b->GetColor();
	m_curveColor[0] = col.r;
	m_curveColor[1] = col.g;
	m_curveColor[2] = col.b;
	if (ImGui::ColorEdit3("Color", &m_curveColor.r)) {
		b->SetColor(m_curveColor);
	}

	// operations
	if (ImGui::Button("Elevate degree")) {
		b->Elevate();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reduce degree")) {
		b->Reduce();
	}
	ImGui::SliderFloat("Cut param", &m_bezierCutParam, 0, 1, "%.2f");
	ImGui::SameLine();
	if (ImGui::Button("Cut")) {
		BezierCurve* newBezier = nullptr;
		b->Cut(m_bezierCutParam, newBezier);

		if (newBezier != nullptr) {
			models->push_back(newBezier);
		}
		else {
			Log::errorToConsole("Unable to cut Bezier-curve");
		}
	}

	ImGui::Separator();
	ImGui::Spacing();
}
/*
void RenderShadow(RenderParams* p, Light* l) {
	return;
}
*/

void BezierCurve::Elevate() {
	std::vector<glm::vec4> newCtrlPoints{};
	int n = GetCtrlPoints().size() - 1;		// degree of the original curve
	for (int k = 0; k <= n + 1; ++k) {
		float alpha = (float)k / ((float)n + 1.f);
		int index1 = glm::min(glm::max(k, 0), n);
		int index2 = glm::min(glm::max(k - 1, 0), n);
		newCtrlPoints.push_back((1 - alpha) * GetCtrlPoints()[index1] + alpha * GetCtrlPoints()[index2]);
	}
	m_ctrlPoints = newCtrlPoints;
	m_ctrlPointsDirty = true;
}
void BezierCurve::Reduce() {
	if (GetCtrlPoints().size() < 3) {
		Log::errorToConsole("Berier::Reduce curve cannot be reduced, too few control points");
		return;
	}
	Log::logToConsole("This method is yet to be implemented");
}
void BezierCurve::Cut(float t, BezierCurve*& newCurve2) {
	if (t < 0 || t > 1) {
		Log::errorToConsole("Bezier::Cut invalid t param");
		return;
	}

	int n = GetCtrlPoints().size() - 1;
	std::vector<glm::vec4> col = GetCtrlPoints();
	std::vector<glm::vec4> newCtrlPs1{};
	std::vector<glm::vec4> newCtrlPs2{};
	for (int i = 0; i <= n; ++i) {
		// set new ctrl points
		newCtrlPs1.push_back(col[0]);
		newCtrlPs2.push_back(col[n - i]);
		// calculate new column
		for (int j = 0; j < n - i; ++j) {
			col[j] = (1 - t) * col[j] + t * col[j + 1];
		}
	}

	// set this curve as first
	SetCtrlPoints(newCtrlPs1);

	// create second curve
	std::stringstream name;
	name << GetName() << "_2";
	newCurve2 = new BezierCurve(BezierCurveParams{
		GetProgramID(),
		GetProgramSelectedID(),
		GetSmoothness(),
		name.str().c_str(),
		true
		});
	std::reverse(newCtrlPs2.begin(), newCtrlPs2.end());
	newCurve2->SetCtrlPoints(newCtrlPs2);
	newCurve2->SetTransforms(GetTransforms());
	return;
}