#include "../../Headers/include_all.h"

void Mesh::Build(std::vector<Vertex> verteces, std::vector<GLuint> indeces) {
	MeshObject<Vertex> mesh{verteces, indeces};
	m_GPU = CreateGLObjectFromMesh(mesh, vertexAttribList);
}

void Mesh::Render(MeshRenderParams* p) {
	// -- Check if the model can be rendered --
	if (GetMaterial() == nullptr) {
		Log::errorToConsole("Corrupted material found");
		exit(1);
	}

	// -- Set render options --
	bool cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
	GLfloat defLineWidth;
	glGetFloatv(GL_LINE_WIDTH, &defLineWidth);
	GLint polygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, polygonMode);
	if (p->wireframe) {
		glDisable(GL_CULL_FACE);
		glLineWidth(p->lineWidth);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// -- Activate shader --
	glUseProgram(p->progID);

	// -- Set shader input data --
	// Layout for model
	glBindVertexArray(GetVAO());

	// Camera module
	glUniform3fv(ul(p->progID, "cameraData.eye"), 1, glm::value_ptr(p->cameraPos));
	glUniformMatrix4fv(ul(p->progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	// Click handler module
	// SSBO bind globally to binding point 0
	glUniform1i(ul(p->progID, "clickHandlerData.modelID"), p->modelIndex);
	glUniform2iv(ul(p->progID, "clickHandlerData.cursorPos"), 1, glm::value_ptr(p->cursorPos));
	glUniform2iv(ul(p->progID, "clickHandlerData.windowSize"), 1, glm::value_ptr(p->windowSize));
	// Material module
	Material::UploadMaterialToShader(p->progID, GetMaterial());
	// Light module
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, p->lights);
	glUniform1i(ul(p->progID, "lightData.lightCount"), p->lightCount);
	// Transform module
	if (p->applyTransforms) {
		glUniformMatrix4fv(ul(p->progID, "transformData.world"), 1, GL_FALSE, glm::value_ptr(p->transform));
	}
	else {
		glm::mat4 world = glm::identity<glm::mat4>();
		glUniformMatrix4fv(ul(p->progID, "transformData.world"), 1, GL_FALSE, glm::value_ptr(world));
	}

	// -- Draw call --
	glDrawElements(p->drawMode, GetVertexCount(), GL_UNSIGNED_INT, nullptr);

	// -- Restore initial OGL state --
	if (cullFaceEnabled) glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, polygonMode[0]);
	glPolygonMode(GL_BACK, polygonMode[1]);
	glLineWidth(defLineWidth);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}
void Mesh::RenderSelection(MeshRenderSelectionParams* p) {
	// -- Set render options --
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(p->selectionWidth);
	GLint polygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, polygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// -- Activate shader --
	glUseProgram(p->progID);

	// -- Set shader input data --
	// Layout for model
	glBindVertexArray(GetVAO());
	// Camera module
	glUniform3fv(ul(p->progID, "cameraData.eye"), 1, glm::value_ptr(p->cameraPos));
	glUniformMatrix4fv(ul(p->progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	// Material module
	Material::UploadMaterialToShader(p->progID, GetMaterial());
	// Transform module
	if (p->applyTransforms) {
		glUniformMatrix4fv(ul(p->progID, "transformData.world"), 1, GL_FALSE, glm::value_ptr(p->transform));
	}
	else {
		glm::mat4 world = glm::identity<glm::mat4>();
		glUniformMatrix4fv(ul(p->progID, "transformData.world"), 1, GL_FALSE, glm::value_ptr(world));
	}
	// Color module
	glUniform3fv(ul(p->progID, "colorData.color"), 1, glm::value_ptr(p->selectionColor));

	// -- Draw call --
	glDrawElements(p->drawMode, GetVertexCount(), GL_UNSIGNED_INT, nullptr);

	// -- Restore initial OGL state --
	glPolygonMode(GL_FRONT, polygonMode[0]);
	glPolygonMode(GL_BACK, polygonMode[1]);
	glLineWidth(lineWidth);
	glUseProgram(0);
	glBindVertexArray(0);
}