#pragma once

#include "include_all.h"

// Light
class Light {
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

	void inline SetType(int newType) {
		type = static_cast<GLfloat>(newType);
	}
	int inline GetType() const {
		return static_cast<int>(type);
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