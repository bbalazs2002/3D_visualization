#pragma once

#include "../include_all.h"

class DiscreteCurve : public ModelBase {
protected:
	std::vector<glm::vec4> m_ctrlPoints{};
	GLuint m_ctrlPointsSSBOID = 0;
	bool m_ctrlPointsDirty = false;
	glm::vec3 m_color = { 1.f, 0, 1.f };

	void SetCtrlPointsSSBO() {
		glGenBuffers(1, &m_ctrlPointsSSBOID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ctrlPointsSSBOID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ctrlPointsSSBOID);
		WriteCtrlPointsSSBO();
	}

	void WriteCtrlPointsSSBO() {
		// apply world transformation on 
		std::vector<glm::vec4> newPoints;
		for (int i = 0; i < m_ctrlPoints.size(); ++i) {
			if (m_applyTransforms) {
				newPoints.push_back(GetTransform() * m_ctrlPoints[i]);
			}
			else {
				newPoints.push_back(m_ctrlPoints[i]);
			}
		}
		// write to buffer
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ctrlPointsSSBOID);
		glBufferData(GL_SHADER_STORAGE_BUFFER,
			newPoints.size() * sizeof(glm::vec4),
			newPoints.data(),
			GL_STATIC_DRAW);
	}

public:
	DiscreteCurve(DiscreteCurveParams params);
	~DiscreteCurve();

	void Render(RenderParams* p) override;
	void RenderSelection(RenderParams* p) override;
	void RenderGUI(std::vector<ModelBase*>* models) override;
	/*
	void RenderShadow(RenderParams* p, Light* l) {
		return;
	}
	*/

	// prevent GetTransform to reset m_transformDirty and Transformation::m_dirty
	inline glm::mat4 GetTransform() const {
		return m_transform;
	}

	inline void AddCtrlPoint(glm::vec3 newPoint) {
		m_ctrlPoints.push_back(glm::vec4(newPoint, 1));
		m_ctrlPointsDirty = true;
	}
	inline void DelCtrlPoint(int index) {
		if (index < 0 || index > m_ctrlPoints.size() - 1) {
			Log::errorToConsole("Bezier::DelCtrlPoint invalid index");
			return;
		}
		m_ctrlPoints.erase(m_ctrlPoints.begin() + index);
		m_ctrlPointsDirty = true;
	}
	inline void SetCtrlPoint(int index, glm::vec3 position) {
		if (index < 0 || index > m_ctrlPoints.size() - 1) {
			Log::errorToConsole("Bezier::SetCtrlPoint invalid index");
			return;
		}
		m_ctrlPoints[index] = glm::vec4(position, 1);
		m_ctrlPointsDirty = true;
	}
	inline void SetCtrlPoints(std::vector<glm::vec4> points) {
		m_ctrlPoints = points;
		m_ctrlPointsDirty = true;
	}
	inline std::vector<glm::vec4> GetCtrlPoints() const {
		return m_ctrlPoints;
	}
	inline GLuint GetCtrlPointsSSBO() const {
		return m_ctrlPointsSSBOID;
	}
	inline void SetApplyTransforms(bool apply) {
		m_applyTransforms = apply;
		m_transformDirty = true;
	}
	inline void SetColor(glm::vec3 color) {
		m_color = color;
	}
	inline void SetColor(float* color) {
		m_color = glm::vec3(color[0], color[1], color[2]);
	}
	inline void SetColor(float r, float g, float b) {
		m_color = glm::vec3(r, g, b);
	}
	inline glm::vec3 GetColor() const {
		return m_color;
	}

};