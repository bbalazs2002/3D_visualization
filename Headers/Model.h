#pragma once

#include "include_all.h"

class Model : public ModelBase {
protected:
	std::vector<Material*> m_materials;
	std::vector<Mesh*> m_meshes;
	bool m_wireframe = false;
	std::string m_objPath;

public:
	char m_objPathBuffer[256] = "";

	Model(ModelParams params);
	~Model();

	void Render(RenderParams* p) override;
	void RenderSelection(RenderParams* p) override;
	void RenderGUI(std::vector<ModelBase*>*) override;

	inline void AddMaterial(Material* material) {
		m_materials.push_back(material);
	}
	inline Material* GetMaterial(int id) const {
		return m_materials[id];
	}
	inline void AddMesh(Mesh* mesh) {
		m_meshes.push_back(mesh);
	}
	inline void SetWireFrame(bool wireframe) {
		m_wireframe = wireframe;
	}
	inline bool GetWireFrame() const {
		return m_wireframe;
	}
	inline void SetObjPath(const char* path) {
		std::strcpy(m_objPathBuffer, path);
		SetObjPath();
	}
	void SetObjPath();

	inline void CleanGeometry() {
		for (Mesh* p : m_meshes) {
			delete(p);
		}
		m_meshes.erase(m_meshes.begin(), m_meshes.end());
		for (Material*  p : m_materials) {
			delete(p);
		}
		m_materials.erase(m_materials.begin(), m_materials.end());
	}
};