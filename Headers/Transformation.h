#pragma once

#include "include_all.h"

class Transformation {
private:
	glm::mat4 m_transformation;
	glm::mat4 m_default;
	bool m_dirty = true;
public:
	Transformation(glm::mat4 transformation) {
		m_transformation = transformation;
		m_default = m_transformation;
	}
	Transformation() : Transformation(glm::identity<glm::mat4>()) {}

	glm::mat4 Get() const {
		return m_transformation;
	}
	glm::mat4 GetDefault() const {
		return m_default;
	}

	void Set(glm::mat4 transformation) {
		m_dirty = true;
		m_transformation = transformation;
	}
	void Set(Transformation transformation) {
		Set(transformation.Get());
	}
	void Set(Transformation* transformation) {
		Set(transformation->Get());
	}

	void UpdateL(glm::mat4 transformation) {
		m_dirty = true;
		m_transformation = transformation * m_transformation;
	}
	void UpdateL(Transformation transformation) {
		UpdateL(transformation.Get());
	}
	void UpdateL(Transformation* transformation) {
		UpdateL(transformation->Get());
	}
	void UpdateR(glm::mat4 transformation) {
		m_dirty = true;
		m_transformation *= transformation;
	}
	void UpdateR(Transformation transformation) {
		UpdateR(transformation.Get());
	}
	void UpdateR(Transformation* transformation) {
		UpdateR(transformation->Get());
	}

	bool IsDirty() const {
		return m_dirty;
	}
	void Clean() {
		m_dirty = false;
	}
};