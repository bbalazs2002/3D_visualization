#pragma once

#include "include_all.h"

struct Material {
public:
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

    static GLuint LoadTexture(const std::filesystem::path& path, bool flip = false) {
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

        return texID;
    }
    // Feltételezzük, hogy a shader program már aktív (glUseProgram(programID))
    static inline void UploadMaterialToShader(GLuint programID, const Material* material) {

        // Shader (fragment shader például)
        // struct Material {
        //     vec3 diffuseColor;
        //     float shininess;
        //     vec3 specularColor;
        //     int hasDiffuseTex;
        //     vec3 ambientColor;
        //     int hasSpecularTex;
        //     int hasEmissionTex;
        //     int hasNormalTex;
        //     sampler2D diffuseTex;
        //     sampler2D specularTex;
        //     sampler2D emissionTex;
        //     sampler2D normalTex;
        // };
        // uniform Material material;

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

        // Színek és fényesség feltöltése
        setVec3("material.diffuseColor", material->diffuseColor);
        setVec3("material.specularColor", material->specularColor);
        setVec3("material.ambientColor", material->ambientColor);
        setFloat("material.shininess", material->shininess);

        // Textúrák hozzárendelése (ha vannak)
        if (material->diffuseTex > 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material->diffuseTex);
            setInt("material.diffuseTex", 0);
            setInt("material.hasDiffuseTex", 1);
        }
        else
        {
            setInt("material.hasDiffuseTex", 0);
        }
        if (material->specularTex > 0) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, material->specularTex);
            setInt("material.specularTex", 1);
            setInt("material.hasSpecularTex", 1);
        }
        else
        {
            setInt("material.hasSpecularTex", 0);
        }
        if (material->emissionTex > 0) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, material->emissionTex);
            setInt("material.emissionTex", 2);
            setInt("material.hasEmissionTex", 1);
        }
        else
        {
            setInt("material.hasEmissionTex", 0);
        }
        if (material->normalTex > 0) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, material->normalTex);
            setInt("material.normalTex", 2);
            setInt("material.hasNormalTex", 1);
        }
        else
        {
            setInt("material.hasNormalTex", 0);
        }
    }

};