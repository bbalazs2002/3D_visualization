#pragma once

#include "../include_all.h"

class DirectionalLight : public Light {
protected:
	glm::vec3 m_direction = glm::vec3(0.0, -1.0, 0.0);

public:
	~DirectionalLight() {
		if (GetCastShadow()) {
			ShadowMapController::Release2DLayer(GetShadowLayer());
		}
	}

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

	inline void SetShadow() override {
		if (m_castShadow) {
			return;
		}
		DirtySSBO();
		m_castShadow = true;
		ShadowMapController::Reserve2DLayer(&m_shadowLayer, &m_lightSpaceMatIndex);
	}
	inline void ClearShadow() override {
		if (!m_castShadow) {
			return;
		}
		DirtySSBO();
		m_castShadow = false;
		ShadowMapController::Release2DLayer(m_shadowLayer);
		m_shadowLayer = 0;
	}

	// IDrawable methods
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
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, Light::GetLightsSSBO());
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

		// Shadow map
		if (GetCastShadow()) {
			GLuint shadowMapID = ShadowMapController::Get2DLayerTextureView(GetShadowLayer());
			if (shadowMapID > 0) {
				ImGui::Image(shadowMapID, ImVec2(150.f, 150.f));
			}
		}
	}

	// parent class methods
	void inline UploadToSSBO(GLuint lightsSSBOID, int padding, GLuint lightSpaceSSBOID) override {
		//struct Light {
		//    vec4 La_const;					// xyz: La, w: constant attenuation
		//    vec4 Ld_linear;					// xyz: Ld, w: linear attenuation
		//    vec4 Ls_quadratic;				// xyz: Ls, w: quadratic attenuation
		//    vec4 direction_lightSpace;		// xyz: direction, w: lightSpaceMatrix
		//    vec4 position;					// xyz: position, w: padding
		//    vec4 flags_angle_plane_shadow;	// x: type, y: inner angle, z: outer angle, w: shadowLayer
		//};

		//
		// 1. Map Lights SSBO
		//
		{
			GLfloat* buffer;
			if (!Light::MapLightsSSBO(padding, buffer)) {
				Log::errorToConsole("Unable to map lights SSBO");
				exit(1);
			}
			//
			// 2. Fill SSBO
			//
			
			// [0-3] La_const (La.xyz és constant attenuation.w)
			memcpy(&buffer[0], glm::value_ptr(GetLa()), sizeof(glm::vec3));

			// [4-7] Ld_linear (Ld.xyz és linear attenuation.w)
			memcpy(&buffer[4], glm::value_ptr(GetLd()), sizeof(glm::vec3));

			// [8-11] Ls_quadratic (Ls.xyz és quadratic attenuation.w)
			memcpy(&buffer[8], glm::value_ptr(GetLs()), sizeof(glm::vec3));

			// [12-15] direction (direction.xyz és lightSpaceMatrix.w)
			memcpy(&buffer[12], glm::value_ptr(GetDirection()), sizeof(glm::vec3));
			buffer[15] = GetLightSpaceMatIndex();

			// [16-19] position (position.xyz és padding.w)

			// [20-23] flags_angle_plane_shadow (flags.x, inner.y, outer.z, shadowLayer.w)
			GLuint flags = LIGHT_FLAG_IS_DIR;
			if (GetCastShadow()) {
				flags = flags |= LIGHT_FLAG_CASTS_SHADOW;
			}
			buffer[20] = flags;
			buffer[23] = GetShadowLayer();

			//
			// 3. Unmap Lights SSBO
			//
			Light::UnmapLightsSSBO();
		}

		//
		// 4. Map LightSpace SSBO
		//
		if (GetCastShadow()) {
			GLfloat* buffer;
			if (!ShadowMapController::Map2DLightSpaceSSBO(m_lightSpaceMatIndex, buffer)) {
				Log::errorToConsole("Unable to map light viewProj matrices SSBO");
				exit(1);
			}
			
			// 5. Upload lightSpaceMatrix (mat4)
			glm::vec3 up{ 0,1,0 };
			float dot = glm::dot(GetDirection(), up);
			if (abs(dot) > .8f) {		// direction and word up is nearly paralell
				up = glm::vec3(0, 0, 1);
			}
			glm::mat4 light_proj = glm::ortho<float>(-10, 10, -10, 10, -10, 10);
			glm::mat4 light_view = glm::lookAt<float>(glm::vec3(0, 0, 0), glm::normalize(GetDirection()), up);
			glm::mat4 light_mvp = light_proj * light_view;

			memcpy(&buffer[0], glm::value_ptr(light_mvp), sizeof(glm::mat4));

			// 6. Unmap LightSpace SSBO
			ShadowMapController::UnmapLightSpaceSSBO();
		}
	}
};