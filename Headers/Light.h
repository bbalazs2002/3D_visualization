#pragma once

#include "include_all.h"

// Light
class Light : public IDrawable {
public:
	glm::vec3 direction = glm::vec3(0.0, 1.0, 0.0);
	glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
	GLfloat constantAttenuation = 1.f;
	glm::vec3 La = glm::vec3(0.2, 0.2, 0.2);
	GLfloat linearAttenuation = 0;
	glm::vec3 Ld = glm::vec3(1.0, 1.0, 1.0);
	GLfloat quadraticAttenuation = 0;
	glm::vec3 Ls = glm::vec3(0.5, 0.5, 0.5);
	GLfloat type = 0;	// directional: 0, point: 1, spot: 2
	GLfloat innerAngle = 0.f;
	GLfloat outerAngle = 0.f;

	// parameters for rendering
	static inline GLuint directionProgram = 0;
	static inline GLuint pointProgram = 0;
	static inline GLuint spotProgram = 0;
	static inline bool show = true;

	void inline SetType(int newType) {
		type = static_cast<GLfloat>(newType);
	}
	int inline GetType() const {
		return static_cast<int>(type);
	}

	void inline Render(RenderParams* p) {
		if (!show) {
			return;
		}

		// -- Activate shader --
		int pointCount = 0;
		GLuint progID = 0;
		if (type > 1.5f) {			// render spot
			progID = spotProgram;
			if (spotProgram <= 0) {
				Log::errorToConsole("Shader for rendering spot lightsource is not found");
			}
		}
		else if (type > .5f) {		// render point
			progID = pointProgram;
			if (pointProgram <= 0) {
				Log::errorToConsole("Shader for rendering point lightsource is not found");
			}
		}
		else {						// render direction
			progID = directionProgram;
			if (directionProgram <= 0) {
				Log::errorToConsole("Shader for rendering direction lightsource is not found");
			}
		}
		if (progID <= 0) {
			show = false;
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
		glUniform1i(ul(progID, "lightData.lightCount"), 1);
		// Other data
		glUniform1i(ul(progID, "lightID"), p->modelIndex);

		// -- Draw call --
		if (type > 1.5f) {			// render spot
			glUniform1i(ul(progID, "isInner"), 0);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 22);
			glUniform1i(ul(progID, "isInner"), 1);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 22);
		}
		else if (type > .5f) {		// render point
			glDrawArrays(GL_TRIANGLES, 0, 24);
		}
		else {						// render direction
			glDrawArrays(GL_TRIANGLES, 0, 33);
		}
		

		// -- Restore initial OGL state --
		if (cullFace) {
			glEnable(GL_CULL_FACE);
		}
		glUseProgram(0);
	}
	void inline RenderSelection(RenderParams* p) { return; }
	void inline RenderGUIBase() {
	
	}
	virtual void RenderGUI(std::vector<ModelBase*>* models) {
	
	}

	static void UploadLightToSSBO(GLuint SSBOID, int count, const Light* const* light) {
		//struct Light {
		//    vec4 La_const;			// xyz: La, w: constant attenuation
		//    vec4 Ld_linear;			// xyz: Ld, w: linear attenuation
		//    vec4 Ls_quadratic;		// xyz: Ls, w: quadratic attenuation
		//    vec4 direction;			// xyz: direction, w: padding
		//    vec4 position;			// xyz: position, w: padding
		//    vec4 type_angle;		    // x: type, y: inner angle, z: outer angle, w: padding
		//};
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOID);

		for (int i = 0; i < count; ++i) {
			size_t padding = i * sizeof(glm::vec4) * 6;

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
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
};