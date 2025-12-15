#pragma once

#include "../include_all.h"

// Light
class Light : public IDrawable {
protected:
	LIGHT_TYPE_TYPE m_type = LIGHT_TYPE_UNDEFINED;	// directional: 0, point: 1, spot: 2
	glm::vec3 m_La = glm::vec3(0.2, 0.2, 0.2);
	glm::vec3 m_Ld = glm::vec3(1.0, 1.0, 1.0);
	glm::vec3 m_Ls = glm::vec3(0.5, 0.5, 0.5);
	bool m_show = true;
	bool m_deleteMarker = false;
	GLuint m_programID = 0;

public:

	void inline SetShow(bool show) {
		m_show = show;
	}
	bool inline GetShow() const {
		return m_show;
	}

	void inline SetType(GLint newType) {
		m_type = static_cast<LIGHT_TYPE_TYPE>(newType);
	}
	GLint inline GetType() const {
		return static_cast<GLint>(m_type);
	}
	glm::vec2 inline GetTypeForSSBO() const {
		return glm::vec2(static_cast<GLfloat>(m_type), 0.f);
	}

	void inline SetProgramID(GLuint programID) {
		m_programID = programID;
	}
	GLuint inline GetProgramID() const {
		return m_programID;
	}

	void inline SetLa(glm::vec3 La) {
		m_La = La;
	}
	glm::vec3 inline GetLa() const {
		return m_La;
	}
	void inline SetLd(glm::vec3 Ld) {
		m_Ld = Ld;
	}
	glm::vec3 inline GetLd() const {
		return m_Ld;
	}
	void inline SetLs(glm::vec3 Ls) {
		m_Ls = Ls;
	}
	glm::vec3 inline GetLs() const {
		return m_Ls;
	}

	void inline MarkForDeletion() {
		m_deleteMarker = true;
	}
	bool inline MarkedForDeletion() const {
		return m_deleteMarker;
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

		if (ImGui::Button("Delete Light")) {
			MarkForDeletion();
			return;
		}

	}

	virtual void UploadToSSBO(GLuint SSBOID, int padding) const = 0;
};