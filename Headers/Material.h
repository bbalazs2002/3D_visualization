#pragma once

#include "include_all.h"

struct Material {
	std::string name = "default";

	glm::vec3 ambientColor{ 1.0f };
	glm::vec3 diffuseColor{ 1.0f };
	glm::vec3 specularColor{ 1.0f };

	float shininess = 32.0f;

	GLuint diffuseTex = 0;
	GLuint specularTex = 0;
	GLuint emissionTex = 0;
	GLuint normalTex = 0;

	~Material();

	/**
	 * @brief Loads an image from a file, creates an OpenGL texture object, and configures its parameters.
	 *
	 * This function handles file reading, error checking, texture object generation,
	 * data upload, mipmap creation, and sets standard filtering and wrapping modes.
	 *
	 * @param path The filesystem path to the image file to load.
	 * @param flip Boolean flag to indicate whether the image data should be flipped vertically (e.g., for common image libraries that load origin at top-left).
	 * @return The OpenGL texture ID (GLuint) if successful, or 0 if loading failed or the path was empty.
	 */
	static inline GLuint LoadTexture(const std::filesystem::path& path, bool flip = false) {
		if (path.empty()) return 0;

		Log::logToConsole("Loading texture: ", path.string());

		ImageRGBA img = ImageFromFile(path, flip);
		if (img.width <= 0 || img.height <= 0) {
			Log::errorToConsole("Failed to load texture: ", path);
			return 0;
		}

		GLuint texID;
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height,
			0, GL_RGBA, GL_UNSIGNED_BYTE, img.data());
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);

		return texID;
	}

	/**
	 * @brief Uploads material properties (colors, textures, shininess) to a GLSL shader program.
	 *
	 * This function takes material data, binds its textures to specific texture units,
	 * and sets the corresponding uniform variables in the active shader program.
	 *
	 * It is assumed that the shader program has already been activated via glUseProgram(programID).
	 *
	 * @param programID The OpenGL program ID (handle) of the target shader.
	 * @param material A constant pointer to the Material structure containing material data
	 * (colors, texture IDs, shininess).
	 * @param textureTargets A fixed-size array of 4 pairs, where each pair defines the
	 * (GL_TEXTUREn enum, texture unit index) for Diffuse, Specular,
	 * Emission, and Normal maps, respectively. This allows customizing
	 * which unit index corresponds to which GL_TEXTUREn slot.
	 */
	static inline void UploadMaterialToShader(
		GLuint programID, const Material* material,
		std::array<std::pair<GLuint, GLuint>, 4> textureTargets = {
			std::make_pair(GL_TEXTURE0, 0),
			std::make_pair(GL_TEXTURE1, 1),
			std::make_pair(GL_TEXTURE2, 2),
			std::make_pair(GL_TEXTURE3, 3)
		}
	) {
		// --- Uniform Helper Lambdas ---
		auto setVec4 = [&](const std::string& name, const glm::vec4& v) {
			GLint loc = glGetUniformLocation(programID, name.c_str());
			if (loc >= 0) glUniform4fv(loc, 1, &v[0]);
			};

		auto setFloat = [&](const std::string& name, float value) {
			GLint loc = glGetUniformLocation(programID, name.c_str());
			if (loc >= 0) glUniform1f(loc, value);
			};

		auto setInt = [&](const std::string& name, int value) {
			GLint loc = glGetUniformLocation(programID, name.c_str());
			if (loc >= 0) glUniform1i(loc, value);
			};

		// --- Upload Scalar and Color Uniforms (materialData struct) ---
		setVec4("materialData.diffuseColorTex", glm::vec4(material->diffuseColor, material->diffuseTex));
		setVec4("materialData.specularColorTex", glm::vec4(material->specularColor, material->specularTex));
		setVec4("materialData.ambientColorEmissionTex", glm::vec4(material->ambientColor, material->emissionTex));
		setFloat("materialData.shininess", material->shininess);
		setInt("materialData.hasNormalTex", material->normalTex);

		// --- Texture Binding and Sampler Setup ---
		// 1. Diffuse Map
		if (material->diffuseTex > 0) {
			glActiveTexture(textureTargets[0].first);
			glBindTexture(GL_TEXTURE_2D, material->diffuseTex);
			setInt("materialDiffuseTex", textureTargets[0].second);
		}
		// 2. Specular Map
		if (material->specularTex > 0) {
			glActiveTexture(textureTargets[1].first);
			glBindTexture(GL_TEXTURE_2D, material->specularTex);
			setInt("materialSpecularTex", textureTargets[1].second);
		}
		// 3. Emission Map
		if (material->emissionTex > 0) {
			glActiveTexture(textureTargets[2].first);
			glBindTexture(GL_TEXTURE_2D, material->emissionTex);
			setInt("materialEmissionTex", textureTargets[2].second);
		}
		// 4. Normal Map
		if (material->normalTex > 0) {
			glActiveTexture(textureTargets[3].first);
			glBindTexture(GL_TEXTURE_2D, material->normalTex);
			setInt("materialNormalTex", textureTargets[3].second);
		}

		// Restore the active texture unit to GL_TEXTURE0 for safety after all bindings.
		glActiveTexture(GL_TEXTURE0);
	}

};