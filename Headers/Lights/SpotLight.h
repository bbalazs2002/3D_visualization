#pragma once

#include "../include_all.h"

class SpotLight : public Light {
protected:
	glm::vec3 m_direction = glm::vec3(0.0, -1.0, 0.0);
	glm::vec3 m_position = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 m_attenuation = glm::vec3(1.0, 0.0, 0.0); // constant, linear, quadratic
	glm::vec2 m_angles = glm::vec2(.2f, .3f); // inner, outer
public:
	SpotLight() {
		m_type = LIGHT_TYPE_SPOT;
	}

	void inline SetDirection(glm::vec3 direction) {
		m_direction = direction;
	}
	glm::vec3 inline GetDirection() const {
		return m_direction;
	}

	void inline SetPosition(glm::vec3 position) {
		m_position = position;
	}
	glm::vec3 inline GetPosition() const {
		return m_position;
	}

	void inline SetConstantAttenuation(GLfloat constant) {
		m_attenuation.x = constant;
	}
	GLfloat inline GetConstantAttenuation() const {
		return m_attenuation.x;
	}

	void inline SetLinearAttenuation(GLfloat linear) {
		m_attenuation.y = linear;
	}
	GLfloat inline GetLinearAttenuation() const {
		return m_attenuation.y;
	}

	void inline SetQuadraticAttenuation(GLfloat quadratic) {
		m_attenuation.z = quadratic;
	}
	GLfloat inline GetQuadraticAttenuation() const {
		return m_attenuation.z;
	}

	glm::vec3 inline GetAttenuation() const {
		return m_attenuation;
	}

	void inline SetInnerAngle(GLfloat inner) {
		m_angles.x = inner;
	}
	GLfloat inline GetInnerAngle() const {
		return m_angles.x;
	}

	void inline SetOuterAngle(GLfloat outer) {
		m_angles.y = outer;
	}
	GLfloat inline GetOuterAngle() const {
		return m_angles.y;
	}

	glm::vec2 inline GetAngles() const {
		return m_angles;
	}

	void inline Render(RenderParams* p) override {
		if (!GetShow()) {
			return;
		}

		// -- Activate shader --
		GLuint progID = GetProgramID();
		if (progID <= 0) {
			Log::errorToConsole("Shader for rendering lightsource is not found");
			SetShow(false);
			return;
		}
		glUseProgram(progID);

		// -- Set render options --
		GLboolean cullFace = glIsEnabled(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);

		// -- Extract data from render params --
		RenderLightParams* rlp = (RenderLightParams*)p->otherData;

		// -- Set shader input data --
		// Camera module
		glUniformMatrix4fv(ul(progID, "cameraData.viewProj"), 1, GL_FALSE, glm::value_ptr(p->viewProj));
		glUniform3fv(ul(progID, "cameraData.eye"), 1, glm::value_ptr(p->cameraPos));
		glUniform3fv(ul(progID, "cameraData.at"), 1, glm::value_ptr(rlp->cameraAt));
		glUniform3fv(ul(progID, "cameraData.up"), 1, glm::value_ptr(rlp->cameraUp));
		// Light module
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, p->lights);
		// Other data
		glUniform1i(ul(progID, "lightID"), p->modelIndex);

		// -- Draw call --
		glUniform1i(ul(progID, "isInner"), 0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 22);
		glUniform1i(ul(progID, "isInner"), 1);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 22);

		// -- Restore initial OGL state --
		if (cullFace) {
			glEnable(GL_CULL_FACE);
		}
		glUseProgram(0);
	}
	void inline RenderSelection(RenderParams* p) override {
		return;
	}
	virtual void RenderGUI(std::vector<ModelBase*>* models) override {
		glm::vec3 buffer = GetPosition();
		if (ImGui::SliderFloat3("Position", &buffer.x, -10.f, 10.f)) {
			SetPosition(buffer);
		}

		buffer = GetDirection();
		if (ImGui::SliderFloat3("Direction", &buffer.x, -10.f, 10.f)) {
			SetDirection(buffer);
		}

		ImGui::Text("Attenuation");
		buffer = GetAttenuation();
		if (ImGui::SliderFloat("Constant", &buffer.x, 0.f, 10.f)) {
			SetConstantAttenuation(buffer.x);
		}
		if (ImGui::SliderFloat("Linear", &buffer.y, 0.f, 10.f)) {
			SetLinearAttenuation(buffer.y);
		}
		if (ImGui::SliderFloat("Quadratic", &buffer.z, 0.f, 10.f)) {
			SetQuadraticAttenuation(buffer.z);
		}

		buffer = glm::vec3(GetAngles(), 0);
		if (ImGui::SliderAngle("Inner angle", &buffer.x, 0, 80)) {
			SetInnerAngle(buffer.x);
		}
		if (ImGui::SliderAngle("Outer angle", &buffer.y, 0, 80)) {
			SetOuterAngle(buffer.y);
		}

	}

	void inline UploadToSSBO(GLuint SSBOID, int padding) const override {
		//struct Light {
		//    vec4 La_const;			// xyz: La, w: constant attenuation
		//    vec4 Ld_linear;			// xyz: Ld, w: linear attenuation
		//    vec4 Ls_quadratic;		// xyz: Ls, w: quadratic attenuation
		//    vec4 direction;			// xyz: direction, w: padding
		//    vec4 position;			// xyz: position, w: padding
		//    vec4 type_angle;		    // x: type, y: inner angle, z: outer angle, w: padding
		//};

		//
		// 1. Map SSBO
		//
		size_t lightSize = sizeof(glm::vec4) * 6;
		size_t p = padding * lightSize;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOID);
		GLfloat* buffer = (GLfloat*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, p, lightSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
		if (!buffer) {
			Log::errorToConsole("Failed to map SSBO for writing directional light data");
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			return;
		}

		//
		// 2. Fill SSBO
		//

		// [0-3] La_const (La.xyz és constant attenuation.w)
		memcpy(&buffer[0], glm::value_ptr(GetLa()), sizeof(glm::vec3));
		buffer[3] = GetConstantAttenuation();

		// [4-7] Ld_linear (Ld.xyz és linear attenuation.w)
		memcpy(&buffer[4], glm::value_ptr(GetLd()), sizeof(glm::vec3));
		buffer[7] = GetLinearAttenuation();

		// [8-11] Ls_quadratic (Ls.xyz és quadratic attenuation.w)
		memcpy(&buffer[8], glm::value_ptr(GetLs()), sizeof(glm::vec3));
		buffer[11] = GetQuadraticAttenuation();

		// [12-15] direction (direction.xyz és padding.w)
		memcpy(&buffer[12], glm::value_ptr(GetDirection()), sizeof(glm::vec3));

		// [16-19] position (position.xyz és padding.w)
		memcpy(&buffer[16], glm::value_ptr(GetPosition()), sizeof(glm::vec3));

		// [20-23] type_angle (type.x, inner.y, outer.z, padding.w)
		buffer[20] = (GLfloat)GetType();
		memcpy(&buffer[21], glm::value_ptr(GetAngles()), sizeof(glm::vec2));

		//
		// 3. Unmap SSBO
		//
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
};