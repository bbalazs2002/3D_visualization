#pragma once

#include "include_all.h"

class Mesh {
	private:
		Material* m_material;
		OGLObject m_GPU;

		static inline const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
		{
			{ 0, offsetof(Vertex, position), 3, GL_FLOAT },
			{ 1, offsetof(Vertex, normal), 3, GL_FLOAT },
			{ 2, offsetof(Vertex, texcoord), 2, GL_FLOAT }
		};

	public:
		~Mesh() {
			CleanOGLObject(m_GPU);

			// material is deleted from wrapper class
			m_material = nullptr;
		}

		inline void SetMaterial(Material* material) {
			m_material = material;
		}
		inline Material* GetMaterial() const {
			return m_material;
		}
		inline GLuint GetVAO() const {
			return m_GPU.vaoID;
		}
		inline GLsizei GetVertexCount() const {
			return m_GPU.count;
		}

		void Build(std::vector<Vertex> verteces, std::vector<unsigned int> indeces);

		void Render(MeshRenderParams* p);
		void RenderSelection(MeshRenderSelectionParams* p);
};