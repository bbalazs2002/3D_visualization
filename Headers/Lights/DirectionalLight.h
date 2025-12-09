#pragma once

#include "../include_all.h"

class DirectionalLight : public Light {
protected:
	glm::vec3 m_direction = glm::vec3(0.0, 1.0, 0.0);

public:
	DirectionalLight() {
		m_type = LIGHT_TYPE_DIRECTIONAL;
	}

	void inline Render(RenderParams* p) {
		if (!GetShow()) {
			return;
		}

		// -- Activate shader --
		GLuint progID = GetProgramID();
		if (progID <= 0) {
			Log::errorToConsole("Shader for rendering lightsource is not found");
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
	void inline RenderSelection(RenderParams* p) {
		return;
	}
	void inline RenderGUI(std::vector<ModelBase*>* models) {
		return;
	}

	virtual void UploadToSSBO(GLuint SSBOID, int padding, int count) {
		//struct Light {
		//    vec4 La_const;			// xyz: La, w: constant attenuation
		//    vec4 Ld_linear;			// xyz: Ld, w: linear attenuation
		//    vec4 Ls_quadratic;		// xyz: Ls, w: quadratic attenuation
		//    vec4 direction;			// xyz: direction, w: padding
		//    vec4 position;			// xyz: position, w: padding
		//    vec4 type_angle;		    // x: type, y: inner angle, z: outer angle, w: padding
		//};
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOID);

		size_t p = padding * sizeof(glm::vec4) * 6;

		// La_const.xyz
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding, sizeof(glm::vec3),
			&light[i]->La.x
		);
		// La_const.w
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding + sizeof(glm::vec3), sizeof(GLfloat),
			&light[i]->constantAttenuation
		);

		// Ld_linear.xyz
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding + sizeof(glm::vec4), sizeof(glm::vec3),
			&light[i]->Ld.x
		);
		// Ld_linear.w
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding + sizeof(glm::vec4) + sizeof(glm::vec3), sizeof(GLfloat),
			&light[i]->linearAttenuation
		);

		// Ls_quadratic.xyz
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding + sizeof(glm::vec4) * 2, sizeof(glm::vec3),
			&light[i]->Ls.x
		);
		// Ls_quadratic.w
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding + sizeof(glm::vec4) * 2 + sizeof(glm::vec3), sizeof(GLfloat),
			&light[i]->quadraticAttenuation
		);

		// direction.xyz
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding + sizeof(glm::vec4) * 3, sizeof(glm::vec3),
			&light[i]->direction.x
		);
		// direction.w (padding)

		// position.xyz
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding + sizeof(glm::vec4) * 4, sizeof(glm::vec3),
			&light[i]->position.x
		);
		// position.w (padding)

		// type_angle.x
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding + sizeof(glm::vec4) * 5, sizeof(GLfloat),
			&light[i]->type
			// &Light::testData
		);
		// type_angle.y
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding + sizeof(glm::vec4) * 5 + sizeof(GLfloat), sizeof(GLfloat),
			&light[i]->innerAngle
		);
		// type_angle.z
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER, padding + sizeof(glm::vec4) * 5 + sizeof(GLfloat) * 2, sizeof(GLfloat),
			&light[i]->outerAngle
		);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
};