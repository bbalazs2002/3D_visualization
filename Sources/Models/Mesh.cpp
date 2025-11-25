#include "../../Headers/include_all.h"

void Mesh::Build(std::vector<Vertex> verteces, std::vector<GLuint> indeces) {
	MeshObject<Vertex> mesh{verteces, indeces};
	m_GPU = CreateGLObjectFromMesh(mesh, vertexAttribList);
}

void Mesh::Render(MeshRenderParams* p) {
	if (GetMaterial() == nullptr) {
		Log::errorToConsole("Corrupted material found");
		exit(1);
	}

	// Log::logToConsole("Render mesh; vertex count: ", GetVertexCount());

	bool cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
	GLfloat defLineWidth;
	glGetFloatv(GL_LINE_WIDTH, &defLineWidth);
	if (p->wireframe) {
		glDisable(GL_CULL_FACE);
		glLineWidth(p->lineWidth);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glUseProgram(p->progID);

	// set material
	Material::UploadMaterialToShader(p->progID, GetMaterial());

	// set light
	Light light = Light();
	if (p->lights.size() > 0) {
		light.pos = p->lights[0];
	}
	Light::UploadLightToShader(p->progID, &light);

	// uniforms
	glUniform3fv(ul(p->progID, "cameraPos"), 1, glm::value_ptr(p->cameraPos));
	glUniform1i(ul(p->progID, "modelID"), p->modelIndex);
	glUniform2iv(ul(p->progID, "cursorPos"), 1, glm::value_ptr(p->cursorPos));
	glUniform2iv(ul(p->progID, "windowSize"), 1, glm::value_ptr(p->windowSize));
	glUniformMatrix4fv(ul(p->progID, "viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	if (p->applyTransforms) {
		glUniformMatrix4fv(ul(p->progID, "world"), 1, GL_FALSE, glm::value_ptr(p->transform));
	}
	else {
		glm::mat4 world = glm::identity<glm::mat4>();
		glUniformMatrix4fv(ul(p->progID, "world"), 1, GL_FALSE, glm::value_ptr(world));
	}

	// bind VAO
	glBindVertexArray(GetVAO());

	// draw call
	glDrawElements(p->drawMode, GetVertexCount(), GL_UNSIGNED_INT, nullptr);

	// restore initial OGL state
	if (cullFaceEnabled) glEnable(GL_CULL_FACE);
	glLineWidth(defLineWidth);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}
void Mesh::RenderSelection(MeshRenderSelectionParams* p) {
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(p->selectionWidth);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUseProgram(p->progID);

	glUniformMatrix4fv(ul(p->progID, "viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
	if (p->applyTransforms) {
		glUniformMatrix4fv(ul(p->progID, "world"), 1, GL_FALSE, glm::value_ptr(p->transform));
	}
	else {
		glm::mat4 world = glm::identity<glm::mat4>();
		glUniformMatrix4fv(ul(p->progID, "world"), 1, GL_FALSE, glm::value_ptr(world));
	}
	glUniform3fv(ul(p->progID, "selColor"), 1, glm::value_ptr(p->selectionColor));

	glBindVertexArray(GetVAO());
	glDrawElements(p->drawMode, GetVertexCount(), GL_UNSIGNED_INT, nullptr);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(lineWidth);
	glUseProgram(0);
	glBindVertexArray(0);
}