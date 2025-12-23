#pragma once

#include "../include_all.h"

class DirectionalLight : public Light {
protected:
	glm::vec3 m_direction = glm::vec3(0.0, 1.0, 0.0);

public:
	void inline SetDirection(glm::vec3 direction) {
		if (m_direction == direction) {
			return;
		}
		DirtySSBO();
		m_direction = direction;
	}
	glm::vec3 inline GetDirection() const {
		return m_direction;
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
		glDrawArrays(GL_TRIANGLES, 0, 33);

		// -- Restore initial OGL state --
		if (cullFace) {
			glEnable(GL_CULL_FACE);
		}
		glUseProgram(0);
	}
	void inline RenderSelection(RenderParams* p) override {
		return;
	}
	void inline RenderGUI(std::vector<ModelBase*>* models) override {
		glm::vec3 dir = GetDirection();
		if (ImGui::SliderFloat3("Direction", &dir.x, -10.f, 10.f)) {
			SetDirection(dir);
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
		memcpy(&buffer[0], glm::value_ptr(glm::identity<glm::mat4>()), sizeof(glm::mat4));

		// [16-19] La_const (La.xyz és constant attenuation.w)
		memcpy(&buffer[16], glm::value_ptr(GetLa()), sizeof(glm::vec3));
		buffer[16] = 0;

		// [20-23] Ld_linear (Ld.xyz és linear attenuation.w)
		memcpy(&buffer[20], glm::value_ptr(GetLd()), sizeof(glm::vec3));
		buffer[23] = 0;

		// [24-27] Ls_quadratic (Ls.xyz és quadratic attenuation.w)
		memcpy(&buffer[24], glm::value_ptr(GetLs()), sizeof(glm::vec3));
		buffer[27] = 0;

		// [28-31] direction (direction.xyz és padding.w)
		memcpy(&buffer[28], glm::value_ptr(GetDirection()), sizeof(glm::vec3));
		buffer[31] = 0;

		// [32-35] position (position.xyz és padding.w)
		memcpy(&buffer[32], glm::value_ptr(glm::vec3(0,0,0)), sizeof(glm::vec3));
		buffer[35] = 0;

		// [36-39] type_angle (flags.x, inner.y, outer.z, shadowLayer.w)
		GLuint flags = LIGHT_FLAG_IS_DIR;
		buffer[36] = flags;
		memcpy(&buffer[37], glm::value_ptr(glm::vec2(0,0)), sizeof(glm::vec2));
		buffer[39] = 0;

		//
		// 3. Unmap SSBO
		//
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
};