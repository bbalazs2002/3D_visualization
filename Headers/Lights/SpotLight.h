#pragma once

#include "../include_all.h"

class SpotLight : public Light {
protected:
	glm::vec3 m_direction = glm::vec3(0.0, -1.0, 0.0);
	glm::vec3 m_position = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 m_attenuation = glm::vec3(1.0, 0.0, 0.0); // constant, linear, quadratic
	glm::vec2 m_angles = glm::vec2(.2f, .3f); // inner, outer
	float m_nearPlane = .5f;
	float m_farPlane = 100.f;
	bool m_farPlaneAuto = false;

public:
	SpotLight() {}

	void inline SetDirection(glm::vec3 direction) {
		if (direction == m_direction) {
			return;
		}
		DirtySSBO();
		m_direction = direction;
	}
	glm::vec3 inline GetDirection() const {
		return m_direction;
	}

	void inline SetPosition(glm::vec3 position) {
		if (position == m_position) {
			return;
		}
		DirtySSBO();
		m_position = position;
	}
	glm::vec3 inline GetPosition() const {
		return m_position;
	}

	void inline SetConstantAttenuation(GLfloat constant) {
		if (m_attenuation.x == constant) {
			return;
		}
		DirtySSBO();
		m_attenuation.x = constant;
	}
	GLfloat inline GetConstantAttenuation() const {
		return m_attenuation.x;
	}
	void inline SetLinearAttenuation(GLfloat linear) {
		if (m_attenuation.y == linear) {
			return;
		}
		DirtySSBO();
		m_attenuation.y = linear;
	}
	GLfloat inline GetLinearAttenuation() const {
		return m_attenuation.y;
	}
	void inline SetQuadraticAttenuation(GLfloat quadratic) {
		if (m_attenuation.z == quadratic) {
			return;
		}
		DirtySSBO();
		m_attenuation.z = quadratic;
	}
	GLfloat inline GetQuadraticAttenuation() const {
		return m_attenuation.z;
	}
	glm::vec3 inline GetAttenuation() const {
		return m_attenuation;
	}

	void inline SetNearPlane(float nearPlane) {
		if (m_nearPlane == nearPlane) {
			return;
		}
		DirtySSBO();
		m_nearPlane = nearPlane;
	}
	float inline GetNearPlane() const {
		return m_nearPlane;
	}
	void inline SetFarPlane(float farPlane) {
		if (m_farPlane == farPlane) {
			return;
		}
		DirtySSBO();
		m_farPlane = farPlane;
	}
	float inline GetFarPlane() const {
		return m_farPlane;
	}
	void inline SetFarPlaneAuto(bool autoCalc) {
		if (m_farPlaneAuto == autoCalc) {
			return;
		}
		DirtySSBO();
		m_farPlaneAuto = autoCalc;
	} 
	bool inline GetFarPlaneAuto() const {
		return m_farPlaneAuto;
	}

	void inline SetInnerAngle(GLfloat inner) {
		if (m_angles.x == inner) {
			return;
		}
		DirtySSBO();
		m_angles.x = inner;
	}
	GLfloat inline GetInnerAngle() const {
		return m_angles.x;
	}
	void inline SetOuterAngle(GLfloat outer) {
		if (m_angles.y == outer) {
			return;
		}
		DirtySSBO();
		m_angles.y = outer;
	}
	GLfloat inline GetOuterAngle() const {
		return m_angles.y;
	}
	glm::vec2 inline GetAngles() const {
		return m_angles;
	}

	float CalculateFarPlane() {
		glm::vec3 combined = GetLd() + GetLs();
		float I0 = std::max({ combined.r, combined.g, combined.b });
		const float minI = 0.01f;

		float a = GetQuadraticAttenuation();
		float b = GetLinearAttenuation();
		float c = GetConstantAttenuation() - (I0 / minI);

		float distance = 100.0f; // Alapértelmezett bukóérték

		// EPSILON vizsgálat a 0-val való osztás elkerülésére
		if (std::abs(a) < 0.00001f) {
			// Elsõfokú egyenlet: bx + c = 0  => x = -c / b
			if (std::abs(b) > 0.00001f) {
				distance = -c / b;
			}
			else {
				// Ha a és b is 0, akkor a fény nem gyengül, 
				// ilyenkor egy fix nagy távolságot kell adni.
				distance = 500.0f;
			}
		}
		else {
			// Teljes másodfokú megoldóképlet
			float discriminant = b * b - 4 * a * c;
			if (discriminant >= 0) {
				distance = (-b + std::sqrt(discriminant)) / (2.0f * a);
			}
		}

		// Biztonsági korlátok: ne legyen negatív és ne legyen túl kicsi
		return std::max(1.0f, distance);
	}

	void Render(RenderParams* p) override {
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
	void RenderSelection(RenderParams* p) override {
		return;
	}
	void RenderGUI(std::vector<ModelBase*>* models) override {
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

		float nearPlane = GetNearPlane();
		if (ImGui::SliderFloat("Near plane", &nearPlane, .1f, 5.f, "%.1f")) {
			SetNearPlane(nearPlane);
		}
		float farPlane = GetFarPlane();
		if (ImGui::SliderFloat("Far plane", &farPlane, .1f, 100.f, "%.1f")) {
			SetFarPlane(farPlane);
		}
		bool farPlaneAuto = GetFarPlaneAuto();
		if (ImGui::Checkbox("Auto calculate far plane", &farPlaneAuto)) {
			SetFarPlaneAuto(farPlaneAuto);
		}

	}

	void inline UploadToSSBO(GLuint SSBOID, int padding) override {
		//struct Light {
		//    mat4 lightSpaceMatrix;	// 
		//    vec4 La_const;			// xyz: La, w: constant attenuation
		//    vec4 Ld_linear;			// xyz: Ld, w: linear attenuation
		//    vec4 Ls_quadratic;		// xyz: Ls, w: quadratic attenuation
		//    vec4 direction;			// xyz: direction, w: padding
		//    vec4 position;			// xyz: position, w: padding
		//    vec4 flags_angle_shadow;	// x: type, y: inner angle, z: outer angle, w: shadowLayer
		//};

		CleanSSBO();

		//
		// 1. Map SSBO
		//
		size_t lightSize = sizeof(glm::vec4) * 10;
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

		// [0-15] lightSpaceMatrix
		glm::vec3 up{ 0,1,0 };
		float dot = glm::dot(GetDirection(), up);
		if (abs(dot) > .8f) {		// direction and word up is nearly paralell
			up = glm::vec3(0, 0, 1);
		}
		float fov = GetOuterAngle() * 2.0f;
		float aspect = 1.0f;
		float nearPlane = GetNearPlane();
		float farPlane;
		if (GetFarPlaneAuto()) {
			farPlane = CalculateFarPlane();
		}
		else {
			farPlane = GetFarPlane();
		}
		glm::mat4 light_proj = glm::perspective(fov, aspect, nearPlane, farPlane);
		glm::mat4 light_view = glm::lookAt<float>(GetPosition(), GetPosition() + glm::normalize(GetDirection()), up);
		glm::mat4 light_mvp = light_proj * light_view;

		memcpy(&buffer[0], glm::value_ptr(light_mvp), sizeof(glm::mat4));

		// [16-19] La_const (La.xyz és constant attenuation.w)
		memcpy(&buffer[16], glm::value_ptr(GetLa()), sizeof(glm::vec3));
		buffer[19] = GetConstantAttenuation();

		// [20-23] Ld_linear (Ld.xyz és linear attenuation.w)
		memcpy(&buffer[20], glm::value_ptr(GetLd()), sizeof(glm::vec3));
		buffer[23] = GetLinearAttenuation();

		// [24-27] Ls_quadratic (Ls.xyz és quadratic attenuation.w)
		memcpy(&buffer[24], glm::value_ptr(GetLs()), sizeof(glm::vec3));
		buffer[27] = GetQuadraticAttenuation();

		// [28-31] direction (direction.xyz és padding.w)
		memcpy(&buffer[28], glm::value_ptr(GetDirection()), sizeof(glm::vec3));

		// [32-35] position (position.xyz és padding.w)
		memcpy(&buffer[32], glm::value_ptr(GetPosition()), sizeof(glm::vec3));

		// [36-39] flags_angle_shadow (flags.x, inner.y, outer.z, shadowLayer.w)
		GLuint flags = LIGHT_FLAG_IS_SPOT;
		if (GetCastShadow()) {
			flags = flags | LIGHT_FLAG_CASTS_SHADOW;
		}
		buffer[36] = flags;
		memcpy(&buffer[37], glm::value_ptr(GetAngles()), sizeof(glm::vec2));
		buffer[39] = GetShadowLayer();

		//
		// 3. Unmap SSBO
		//
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
};