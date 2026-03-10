#pragma once

#include "../include_all.h"

// Light
class Light : public IDrawable {
protected:
	glm::vec3 m_La = glm::vec3(0.2, 0.2, 0.2);
	glm::vec3 m_Ld = glm::vec3(1.0, 1.0, 1.0);
	glm::vec3 m_Ls = glm::vec3(0.5, 0.5, 0.5);
	bool m_show = true;
	GLuint m_programID = 0;
	GLuint m_shadowLayer = 0;
	bool m_castShadow = false;
	GLuint m_lightSpaceMatIndex = 0;

	bool m_dirtySSBO = true;

	void CleanSSBO() {
		m_dirtySSBO = false;
	}
	void DirtySSBO() {
		m_dirtySSBO = true;
	}

	// static utils
	static inline GLuint lightsSSBOID = 0;
	static inline std::vector<Light*> lights;

	static inline void AllocateLightsSSBO() {
		if (lightsSSBOID > 0) {
			glDeleteBuffers(1, &lightsSSBOID);
		}

		if (lights.size() > 0) {
			glCreateBuffers(1, &lightsSSBOID);
			glNamedBufferStorage(lightsSSBOID, lights.size() * sizeof(glm::vec4) * 6, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		}
	}

public:
	void inline SetShow(bool show) {
		if (show == m_dirtySSBO) {
			return;
		}
		DirtySSBO();
		m_show = show;
	}
	bool inline GetShow() const {
		return m_show;
	}

	virtual void SetShadow() = 0;
	virtual void ClearShadow() = 0;
	GLuint GetShadowLayer() const {
		return m_shadowLayer;
	}
	bool GetCastShadow() const {
		return m_castShadow;
	}

	void inline SetLightSpaceMatIndex(GLuint index) {
		m_lightSpaceMatIndex = index;
	}
	GLuint inline GetLightSpaceMatIndex() {
		return m_lightSpaceMatIndex;
	}

	void inline SetProgramID(GLuint programID) {
		m_programID = programID;
	}
	GLuint inline GetProgramID() const {
		return m_programID;
	}

	void inline SetLa(glm::vec3 La) {
		if (m_La == La) {
			return;
		}
		DirtySSBO();
		m_La = La;
	}
	glm::vec3 inline GetLa() const {
		return m_La;
	}
	void inline SetLd(glm::vec3 Ld) {
		if (m_Ld == Ld) {
			return;
		}
		DirtySSBO();
		m_Ld = Ld;
	}
	glm::vec3 inline GetLd() const {
		return m_Ld;
	}
	void inline SetLs(glm::vec3 Ls) {
		if (m_Ls == Ls) {
			return;
		}
		DirtySSBO();
		m_Ls = Ls;
	}
	glm::vec3 inline GetLs() const {
		return m_Ls;
	}

	bool IsDirtySSBO() const {
		return m_dirtySSBO;
	}

	virtual void Render(RenderParams* p) = 0;
	virtual void RenderSelection(RenderParams* p) = 0;
	virtual void RenderGUI(std::vector<ModelBase*>* models) = 0;
	void inline RenderGUIBase() {
		glm::vec3 color = GetLa();
		if (ImGui::SliderFloat3("La", &color.x, 0.f, 10.f)) {
			SetLa(color);
		}
		color = GetLd();
		if (ImGui::SliderFloat3("Ld", &color.x, 0.f, 10.f)) {
			SetLd(color);
		}
		color = GetLs();
		if (ImGui::SliderFloat3("Ls", &color.x, 0.f, 10.f)) {
			SetLs(color);
		}
		bool shadow = GetCastShadow();
		if (ImGui::Checkbox("Cast shadow", &shadow)) {
			if (GetCastShadow()) {
				ClearShadow();
			}
			else {
				SetShadow();
			}
		}
	}

	virtual void UploadToSSBO(GLuint lightsSSBOID, int padding, GLuint lightSpaceSSBOID) = 0;

	// static utils
	static inline void AddLight(Light* l) {
		lights.push_back(l);
		AllocateLightsSSBO();
	}
	static inline void DelLight(int index) {
		if (index < 0 || index >= lights.size()) {
			Log::errorToConsole("Light::DelLight invalid index");
			return;
		}
		size_t lastIndex = lights.size() - 1;

		delete(lights[index]);
		if (index != lastIndex) {
			lights[index] = lights[lastIndex];
			lights[index]->DirtySSBO();
		}
		lights.pop_back();

		AllocateLightsSSBO();
	}
	static inline Light* GetLight(int index) {
		if (index < 0 || index >= lights.size()) {
			Log::errorToConsole("Light::GetLight invalid index");
			return nullptr;
		}
		return lights[index];
	}
	static inline size_t GetLightCount() {
		return lights.size();
	}
	static inline GLuint GetLightsSSBO() {
		return lightsSSBOID;
	}
	static inline void UpdateLights() {
		int count = 0;
		for (auto l : lights) {
			if (l->IsDirtySSBO()) {
				l->UploadToSSBO(lightsSSBOID, count, l->GetLightSpaceMatIndex());
				l->CleanSSBO();
			}
			++count;
		}
	}
	static inline bool MapLightsSSBO(size_t padding, GLfloat*& buffer) {
		size_t lightSize = sizeof(glm::vec4) * 6;
		size_t p = padding * lightSize;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsSSBOID);
		buffer = (GLfloat*)glMapNamedBufferRange(lightsSSBOID, p, lightSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
		if (!buffer) {
			Log::errorToConsole("Failed to map lights SSBO");
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			return false;
		}
		return true;
	}
	static inline void BindLightsSSBO(GLint bindingPoint) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, lightsSSBOID);
	}
	static inline void UnmapLightsSSBO() {
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	static inline void RenderSelected(int selected, RenderParams* p) {
		if (selected < 0 || selected >= lights.size()) {
			return;
		}
		lights[selected]->Render(p);
	}
	static void RenderShadowMap(std::vector<ModelBase*>* models);
	static inline void Clean() {
		if (lightsSSBOID > 0) {
			glDeleteBuffers(1, &lightsSSBOID);
			lightsSSBOID = 0;
		}
	}
};