#pragma once

#include "../include_all.h"

class PointLight : public Light {
protected:
	glm::vec3 m_position = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 m_attenuation = glm::vec3(1.0, 0.0, 0.0); // constant, linear, quadratic

	float m_nearPlane = .5f;
	float m_farPlane = 100.f;
	bool m_farPlaneAuto = false;

	static inline int faceIndex = 0;
public:

	~PointLight() {
		if (GetCastShadow()) {
			ShadowMapController::ReleaseCubeLayer(GetShadowLayer());
		}
	}

	void inline SetPosition(glm::vec3 position) {
		if (m_position == position) {
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

	float inline CalculateFarPlane() {
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

	inline void SetShadow() override {
		if (m_castShadow) {
			return;
		}
		DirtySSBO();
		m_castShadow = true;
		ShadowMapController::ReserveCubeLayer(&m_shadowLayer, &m_lightSpaceMatIndex);
	}
	inline void ClearShadow() {
		if (!m_castShadow) {
			return;
		}
		DirtySSBO();
		m_castShadow = false;
		ShadowMapController::ReleaseCubeLayer(m_shadowLayer);
		m_shadowLayer = 0;
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
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, Light::GetLightsSSBO());
		// Other data
		glUniform1i(ul(progID, "lightID"), p->modelIndex);

		// -- Draw call --
		glDrawArrays(GL_TRIANGLES, 0, 24);

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
		glm::vec3 buffer = GetPosition();
		if (ImGui::SliderFloat3("Position", &buffer.x, -10.f, 10.f)) {
			SetPosition(buffer);
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

		// Shadow map
		if (GetCastShadow()) {
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text("Shadow Map");
			ImGui::InputInt("faceIndex", &PointLight::faceIndex);
			GLuint shadowMapID = ShadowMapController::GetCubeLayerTextureView(GetShadowLayer(), PointLight::faceIndex);
			if (shadowMapID > 0) {
				ImGui::Image(shadowMapID, ImVec2(150.f, 150.f));
			}
			else {
//				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
//				ImGui::Text("Shadow map not available for the selected face index.");
//				ImGui::PopStyleColor();

				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Shadow map not available for the selected face index.");
			}
			ImGui::Separator();
			ImGui::Spacing();
		}
		/*
		if (GetCastShadow()) {
			for (int i = 0; i < 6; ++i) {
				GLuint shadowMapID = ShadowMapController::GetCubeLayerTextureView(GetShadowLayer(), i);
				if (shadowMapID > 0) {
					ImGui::Image(shadowMapID, ImVec2(150.f, 150.f));
				}
				if (i != 2 && i < 5) {
					ImGui::SameLine();
				}
			}
		}
		*/
	}

	void inline UploadToSSBO(GLuint SSBOID, int padding, GLuint lightSpaceSSBOID) override {
		//struct Light {
		//    vec4 La_const;					// xyz: La, w: constant attenuation
		//    vec4 Ld_linear;					// xyz: Ld, w: linear attenuation
		//    vec4 Ls_quadratic;				// xyz: Ls, w: quadratic attenuation
		//    vec4 direction_lightSpace;		// xyz: direction, w: lightSpaceMatrix
		//    vec4 position;					// xyz: position, w: padding
		//    vec4 flags_angle_plane_shadow;	// x: type, y: near plane, z: far plane, w: shadowLayer
		//};

		// Define clipping planes.
		float nearPlane = GetNearPlane();
		float farPlane;
		if (GetFarPlaneAuto()) {
			// Automatically calculate the far plane based on light intensity/attenuation.
			farPlane = CalculateFarPlane();
		}
		else {
			// Use a manually defined fixed range.
			farPlane = GetFarPlane();
		}

		{
			//
			// 1. Map SSBO
			//
			GLfloat* buffer;
			Light::MapLightsSSBO(padding, buffer);

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

			// [12-15] direction (direction.xyz és lightSpaceMatrix.w)
			buffer[15] = GetLightSpaceMatIndex();

			// [16-19] position (position.xyz és padding.w)
			memcpy(&buffer[16], glm::value_ptr(GetPosition()), sizeof(glm::vec3));

			// [20-23] flags_angle_plane_shadow (flags.x, near.y, far.z, shadowLayer.w)
			GLuint flags = LIGHT_FLAG_IS_POINT;
			if (GetCastShadow()) {
				flags = flags |= LIGHT_FLAG_CASTS_SHADOW;
			}
			buffer[20] = flags;
			if (GetCastShadow()) {
				buffer[21] = nearPlane;
				buffer[22] = farPlane;
			}
			buffer[23] = GetShadowLayer();

			//
			// 3. Unmap SSBO
			//
			Light::UnmapLightsSSBO();
		}

		if (GetCastShadow()) {
			//
			// 4. Map LightSpace SSBO
			//
			GLfloat* buffer;
			if (!ShadowMapController::MapCubeLightSpaceSSBO(GetLightSpaceMatIndex(), buffer)) {
				return; // Exit if mapping fails
			}
			glm::mat4* mat_buffer = (glm::mat4*)buffer;

			//
			// 5. Upload lightSpaceMatrix (mat4)
			//
			glm::vec3 pos = GetPosition();

			// a) Create the Projection Matrix:
			// - 90-degree Field of View (FOV): Required for the 6 faces to perfectly align into a cube.
			// - 1.0 Aspect Ratio: Shadow map faces (e.g., 512x512) are always square.
			glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);

			// b) Calculate the 6 View Matrices based on the OpenGL CubeMap specification:
			// Note: Due to the internal coordinate system of OpenGL CubeMaps, "Up" vectors differ 
			//       from the standard (0, 1, 0) to ensure textures are not mirrored or flipped.

			// Look towards +X, Up is negative Y - GL_TEXTURE_CUBE_MAP_POSITIVE_X (Right)
			mat_buffer[0] = shadowProj * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));

			// Look towards -X, Up is negative Y - GL_TEXTURE_CUBE_MAP_NEGATIVE_X (Left)
			mat_buffer[1] = shadowProj * glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));

			// Look towards +Y, Up is positive Z - GL_TEXTURE_CUBE_MAP_POSITIVE_Y (Top)
			mat_buffer[2] = shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			// Look towards -Y, Up is negative Z - GL_TEXTURE_CUBE_MAP_NEGATIVE_Y (Bottom)
			mat_buffer[3] = shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

			// Look towards +Z, Up is negative Y - GL_TEXTURE_CUBE_MAP_POSITIVE_Z (Back)
			mat_buffer[4] = shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

			// Look towards -Z, Up is negative Y - GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (Front)
			mat_buffer[5] = shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

			//
			// 6. Unmap LightSpace SSBO
			//
			ShadowMapController::UnmapLightSpaceSSBO();
		}
	}
};