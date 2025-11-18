#pragma once

#include "include_all.h"

// Light
struct Light {
	glm::vec3 pos = glm::vec4(1.0, 1.0, 0.0, 0.0);
	float constantAttenuation = 1.f;
	glm::vec3 La = glm::vec3(0.2, 0.2, 0.2);
	float linearAttenuation = 0;
	glm::vec3 Ld = glm::vec3(1.0, 1.0, 1.0);
	float quadraticAttenuation = 0;
	glm::vec3 Ls = glm::vec3(0.5, 0.5, 0.5);
	int type = 0;

	static void UploadLightToShader(GLuint programID, Light* light) {
        // struct Light {
        //     vec3 pos;
        //     float constantAttenuation;
        //     vec3 La;
        //     float linearAttenuation;
        //     vec3 Ld;
        //     float quadraticAttenuation;
        //     vec3 Ls;
        //     int type;
        // }
        // uniform Light light;

        // Segédfüggvény a uniform helyek gyors eléréséhez
        auto setVec3 = [&](const std::string& name, const glm::vec3& v) {
            GLint loc = glGetUniformLocation(programID, name.c_str());
            if (loc >= 0) glUniform3fv(loc, 1, &v[0]);
            };

        auto setFloat = [&](const std::string& name, float value) {
            GLint loc = glGetUniformLocation(programID, name.c_str());
            if (loc >= 0) glUniform1f(loc, value);
            };

        auto setInt = [&](const std::string& name, int value) {
            GLint loc = glGetUniformLocation(programID, name.c_str());
            if (loc >= 0) glUniform1i(loc, value);
            };

        setVec3("light.pos", light->pos);
        setVec3("light.La", light->La);
        setVec3("light.Ld", light->Ld);
        setVec3("light.Ls", light->Ls);

        setFloat("light.constantAttenuation", light->constantAttenuation);
        setFloat("light.linearAttenuation", light->linearAttenuation);
        setFloat("light.quadraticAttenuation", light->quadraticAttenuation);

        setInt("light.type", light->type);
	}
};