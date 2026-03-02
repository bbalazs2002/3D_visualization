#include "../../Headers/include_all.h"

Model::Model(ModelParams params) : ModelBase(MODEL2MODELBASE) {
	m_wireframe = params.wireFrame;
	m_shadowProgramID = params.shaderPrograms.programShadowID;
}
Model::~Model() {
	CleanGeometry();
	CleanMaterials();
}

// Drawable methods
void Model::Render(RenderParams* p) {
	if (!GetShow()) {
		return;
	}

	glm::mat4 modelTransform = glm::identity<glm::mat4>();
	if (GetApplyTransforms()) {
		modelTransform = GetTransform();
	}

	MeshRenderParams mp{
		GetProgramID(),
		GetDrawMode()
	};

	// -- Set render options --
	bool cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
	GLfloat defLineWidth;
	glGetFloatv(GL_LINE_WIDTH, &defLineWidth);
	GLint polygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, polygonMode);
	if (GetWireFrame()) {
		glDisable(GL_CULL_FACE);
		glLineWidth(p->lineWidth);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// -- Activate shader --
	GLuint progID = GetProgramID();
	glUseProgram(progID);

	// -- Set shader input data --
	// Camera module
	glUniform3fv(ul(progID, "cameraData.eye"), 1, glm::value_ptr(p->cameraPos));
	glUniformMatrix4fv(ul(progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	// Click handler module
	// SSBO bind globally to binding point 0
	glUniform1i(ul(progID, "clickHandlerData.modelID"), p->modelIndex);
	glUniform2iv(ul(progID, "clickHandlerData.cursorPos"), 1, glm::value_ptr(p->cursorPos));
	glUniform2iv(ul(progID, "clickHandlerData.windowSize"), 1, glm::value_ptr(p->windowSize));
	// Light module
	// SSBO bind globally to binding point 2
	// Shadow texture globally uploaded to unit #4
	glUniform1i(ul(progID, "lightShadowMapArray"), 4);
	glUniform1i(ul(progID, "lightData.lightCount"), Light::GetLightCount());
	// Transform module
	glUniformMatrix4fv(ul(progID, "transformData.world"), 1, GL_FALSE, glm::value_ptr(modelTransform));

	if (
		!p->selected ||
		(p->selected && (CMyApp::MeshID < 0 || CMyApp::MeshID >= m_meshes.size()))
		) {
		// Render all meshes
		for (Mesh* mesh : m_meshes) {
			mesh->Render(&mp);
		}
	}
	else {
		// render only one mesh
		Mesh* mesh = m_meshes[CMyApp::MeshID];
		mesh->Render(&mp);
	}

	// -- Restore initial OGL state --
	if (cullFaceEnabled) glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, polygonMode[0]);
	glPolygonMode(GL_BACK, polygonMode[1]);
	glLineWidth(defLineWidth);
	glUseProgram(0);

	if (p->selected && p->selectionWidth > 0) {
		RenderSelection(p);
	}

}
void Model::RenderSelection(RenderParams* p) {
	glm::mat4 modelTransform = glm::identity<glm::mat4>();
	if (GetApplyTransforms()) {
		modelTransform = GetTransform();
	}
	MeshRenderSelectionParams msp{
		GetProgramSelectedID(),
		GetDrawMode()
	};

	// -- Set render options --
	GLfloat defLineWidth;
	glGetFloatv(GL_LINE_WIDTH, &defLineWidth);
	glLineWidth(p->selectionWidth);
	GLint polygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, polygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	bool cullface = glIsEnabled(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	// -- Activate shader --
	GLuint progID = GetProgramSelectedID();
	glUseProgram(progID);

	// -- Set shader input data --
	// Camera module
	glUniform3fv(ul(progID, "cameraData.eye"), 1, glm::value_ptr(p->cameraPos));
	glUniformMatrix4fv(ul(progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	// Transform module
	glUniformMatrix4fv(ul(progID, "transformData.world"), 1, GL_FALSE, glm::value_ptr(modelTransform));
	// Color module
	glUniform3fv(ul(progID, "colorData.color"), 1, glm::value_ptr(p->selectionColor));

	if (CMyApp::MeshID < 0 || CMyApp::MeshID >= m_meshes.size()) {
		// Render all meshes
		for (Mesh* mesh : m_meshes) {
			mesh->RenderSelection(&msp);
		}
	}
	else {
		// render only one mesh
		Mesh* mesh = m_meshes[CMyApp::MeshID];
		mesh->RenderSelection(&msp);
	}

	// -- Restore initial OGL state --
	glLineWidth(defLineWidth);
	if (cullface) glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, polygonMode[0]);
	glPolygonMode(GL_BACK, polygonMode[1]);
	glUseProgram(0);
}
void Model::RenderGUI(std::vector<ModelBase*>*) {
	ImGui::Text("Model specific options");
	ImGui::Spacing();

	Model* m = this;

	// Wireframe
	bool wireframe = m->GetWireFrame();
	if (ImGui::Checkbox("Wireframe", &wireframe)) {
		m->SetWireFrame(wireframe);
	}

	// Shadow
	bool shadow = m->GetCastShadow();
	if (ImGui::Checkbox("Cast shadow", &shadow)) {
		SetCastShadow(shadow);
	}

	// OBJ file
	ImGui::InputText("Obj file path", m->m_objPathBuffer, IM_ARRAYSIZE(m->m_objPathBuffer));
	ImGui::SameLine();
	if (ImGui::Button("Load")) {
		m->SetObjPath();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
}

// ICastShadow methods
void Model::RenderShadowMap(GLint lightID, GLint faceID) {
	if (!GetShow() || GetWireFrame()) {
		return;
	}

	glm::mat4 modelTransform = glm::identity<glm::mat4>();
	if (GetApplyTransforms()) {
		modelTransform = GetTransform();
	}

	MeshRenderShadowParams msp{
		GetDrawMode()
	};

	// -- Set render options --
	bool cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	GLint depthFunc;
	glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
	GLint depthMask;
	glGetIntegerv(GL_DEPTH_WRITEMASK, &depthMask);
	bool depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	// -- Activate shader --
	GLuint progID = GetProgramShadowID();
	glUseProgram(progID);

	// -- Set shader input data --
	// Transform module
	glUniformMatrix4fv(ul(progID, "transformData.world"), 1, GL_FALSE, glm::value_ptr(modelTransform));
	// Light module
	// SSBO bind globally to binding point 2
	// Shader data
	glUniform1i(ul(progID, "lightID"), lightID);
	glUniform1i(ul(progID, "faceID"), faceID);

	if (CMyApp::MeshID < 0 || CMyApp::MeshID >= m_meshes.size()) {
		// Render all meshes
		for (Mesh* mesh : m_meshes) {
			mesh->RenderShadowMap(&msp);
		}
	}
	else {
		// render only one mesh
		Mesh* mesh = m_meshes[CMyApp::MeshID];
		mesh->RenderShadowMap(&msp);
	}

	// -- Restore initial OGL state --
	glUseProgram(0);
	if (cullFaceEnabled) {
		glEnable(GL_CULL_FACE);
	}
	if (depthTestEnabled) {
		glEnable(GL_DEPTH_TEST);
	}
	glDepthFunc(depthFunc);
	glDepthMask(depthMask);
}

void Model::SetObjPath() {
	if (!std::filesystem::exists(m_objPathBuffer)) {
		return;
	}
	std::string tempPath = m_objPathBuffer;
	if (tempPath == m_objPath) {
		return;
	}
	m_objPath = tempPath;

	// clean old geometry
	CleanGeometry();
	CleanMaterials();
	// load the new file
	ModelLoaderReturn meshMatData = ModelLoader::LoadFromOBJ(m_objPath);
	m_meshes = meshMatData.meshes;
	m_materials = meshMatData.materials;
}