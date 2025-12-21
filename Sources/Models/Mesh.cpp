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

	// -- Set shader input data --
	// Layout for model
	glBindVertexArray(GetVAO());
	// Material module
	Material::UploadMaterialToShader(p->progID, GetMaterial());

	// -- Draw call --
	glDrawElements(p->drawMode, GetVertexCount(), GL_UNSIGNED_INT, nullptr);

	// -- Restore initial OGL state --
	glBindVertexArray(0);
	Material::ClearMaterialFromShader();
}
void Mesh::RenderSelection(MeshRenderSelectionParams* p) {
	// -- Set shader input data --
	// Layout for model
	glBindVertexArray(GetVAO());

	// -- Draw call --
	glDrawElements(p->drawMode, GetVertexCount(), GL_UNSIGNED_INT, nullptr);

	// -- Restore initial OGL state --
	glBindVertexArray(0);
}
void Mesh::RenderShadowMap(MeshRenderShadowParams* p) {
	glBindVertexArray(GetVAO());

	// -- Draw call --
	glDrawElements(p->drawMode, GetVertexCount(), GL_UNSIGNED_INT, nullptr);

	// -- Restore initial OGL state --
	glBindVertexArray(0);
}