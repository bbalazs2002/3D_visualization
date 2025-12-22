#pragma once

#include "include_all.h"

struct ShaderProgramCollection {
    GLuint programID = 0;
    GLuint programSelectedID = 0;
    GLuint programShadowID = 0;
};

// Model params
struct BezierCurveParams {
	ShaderProgramCollection shaderPrograms;
    int smoothness = 10;
    const char* name = "";
    bool show = true;
};
struct BezierSurfaceParams {
	ShaderProgramCollection shaderPrograms;
    glm::vec2 smoothness{ 10, 10 };
    const char* name = "";
    bool show = true;
    bool wireframe = false;
};
struct BSplineParams {
	ShaderProgramCollection shaderPrograms;
    int smoothness = 10;
    const char* name = "";
    bool show = true;
};
struct DiscreteCurveParams {
	ShaderProgramCollection shaderPrograms;
    const char* name = "";
    bool show = true;
};
struct ModelParams {
    ShaderProgramCollection shaderPrograms;
    const char* name = "";
    bool show = true;
    bool wireFrame = false;
    int drawMode = GL_TRIANGLES;
};

// Render params
struct MeshRenderParams {
    GLuint progID;
    int drawMode;
};
struct MeshRenderSelectionParams {
    GLuint progID;
    int drawMode;
};
struct MeshRenderShadowParams {
    int drawMode;
};

struct ModelBaseParams {
	ShaderProgramCollection shaderPrograms;
    const char* name = "";
    bool show = true;
    int drawMode = GL_TRIANGLES;
};
struct RenderLightParams {
    glm::vec3 cameraAt{ 0,0,0 };
    glm::vec3 cameraUp{ 0,1,0 };
};
struct RenderParams {
    float lineWidth = 1.f;
    glm::vec3 cameraPos = glm::vec3(0, 0, 0);
    GLuint lights;                                      // SSBO ID for lights
    int lightCount = 0;
    int modelIndex = 0;
    glm::ivec2 cursorPos = glm::ivec2(0, 0);
    glm::ivec2 windowSize = glm::ivec2(0, 0);
    glm::mat4 viewProj = glm::identity<glm::mat4>();
    bool selected = false;
    float selectionWidth = 1.f;
    glm::vec3 selectionColor = glm::vec3(1.f, 0, 0);
    void* otherData = nullptr;
};

// ModelLoader
struct ModelLoaderReturn {
    std::vector<Material*> materials;
    std::vector<Mesh*> meshes;
};

// Update info
struct SUpdateInfo
{
    float ElapsedTimeInSec = 0.0f;	// Elapsed time since start of the program
    float DeltaTimeInSec = 0.0f;	// Elapsed time since last update
};