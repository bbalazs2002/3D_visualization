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

	bool m_dirtySSBO = true;

	void CleanSSBO() {
		m_dirtySSBO = false;
	}
	void DirtySSBO() {
		m_dirtySSBO = true;
	}

public:

	virtual ~Light() {
		if (m_castShadow) {
			ShadowMapController::ReleaseLayer(m_shadowLayer);
		}
	}

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

	void SetShadow() {
		if (m_castShadow) {
			return;
		}
		DirtySSBO();
		m_castShadow = true;
		ShadowMapController::ReserveLayer(&m_shadowLayer);
	}
	void ClearShadow() {
		if (!m_castShadow) {
			return;
		}
		DirtySSBO();
		m_castShadow = false;
		ShadowMapController::ReleaseLayer(m_shadowLayer);
		m_shadowLayer = 0;
	}
	GLuint GetShadowLayer() const {
		return m_shadowLayer;
	}
	bool GetCastShadow() const {
		return m_castShadow;
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

	virtual float CalculateFarPlane() {
		// 1. Kezdeti intenzitás meghatározása
		glm::vec3 combined = GetLd() + GetLs();
		float I0 = std::max({ combined.r, combined.g, combined.b });

		// 2. Küszöbérték (pl. 0.01f a látható tartomány alja)
		const float minI = 0.01f;

		// 3. Másodfokú egyenlet együtthatói
		float a = 0;				// quadraticAttenuation;
		float b = 0;				// linearAttenuation;
		float c = 0 - (I0 / minI);	// constantAttenuation;

		// 4. Megoldóképlet (távolság kiszámítása)
		float distance = 100.0f; // Biztonsági alapérték
		float discriminant = b * b - 4 * a * c;
		if (discriminant >= 0) {
			distance = (-b + std::sqrt(discriminant)) / (2.0f * a);
		}
		return distance;
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

	virtual void UploadToSSBO(GLuint SSBOID, int padding) = 0;
};