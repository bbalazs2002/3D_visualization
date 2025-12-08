#pragma once

#include "include_all.h"

struct ModelBaseParams {
    GLuint programID = 0;
    GLuint programSelectedID = 0;
    const char* name = "";
    bool show = true;
    int drawMode = GL_TRIANGLES;
};

struct ModelParams {
    GLuint programID = 0;
    GLuint programSelectedID = 0;
    const char* name = "";
    bool show = true;
    bool wireFrame = false;
    int drawMode = GL_TRIANGLES;
};

struct BezierCurveParams {
    GLuint programID = 0;
    GLuint programSelectedID = 0;
    int smoothness = 10;
    const char* name = "";
    bool show = true;
};

struct BSplineParams {
    GLuint programID = 0;
    GLuint programSelectedID = 0;
    int smoothness = 10;
    const char* name = "";
    bool show = true;
};

struct DiscreteCurveParams {
    GLuint programID = 0;
    GLuint programSelectedID = 0;
    const char* name = "";
    bool show = true;
};

struct BezierSurfaceParams {
    GLuint programID = 0;
    GLuint programSelectedID = 0;
    glm::vec2 smoothness{ 10, 10 };
    const char* name = "";
    bool show = true;
    bool wireframe = false;
};

// ModelLoader
struct ModelLoaderReturn {
    std::vector<Material*> materials;
    std::vector<Mesh*> meshes;
};

// Render options
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

struct RenderLightParams {
    glm::vec3 cameraAt{0,0,0};
    glm::vec3 cameraUp{0,1,0};
};

struct RenderShadowParams {
    float lineWidth = 1.f;
    glm::mat4 viewProj = glm::identity<glm::mat4>();
};

struct MeshRenderParams {
    float lineWidth;
    glm::vec3 cameraPos;
    GLuint lights;                                      // SSBO ID for lights
    int lightCount;
    int modelIndex;
    glm::ivec2 cursorPos;
    glm::ivec2 windowSize;
    glm::mat4 viewProj;
    //
    GLuint progID;
    bool wireframe;
    bool applyTransforms;
    glm::mat4 transform;
    int drawMode;
};

struct MeshRenderSelectionParams {
    glm::vec3 cameraPos;
    glm::mat4 viewProj;
    float selectionWidth;
    glm::vec3 selectionColor;
    //
    GLuint progID;
    bool applyTransforms;
    glm::mat4 transform;
    int drawMode;
};

struct SUpdateInfo
{
    float ElapsedTimeInSec = 0.0f;	// Elapsed time since start of the program
    float DeltaTimeInSec = 0.0f;	// Elapsed time since last update
};