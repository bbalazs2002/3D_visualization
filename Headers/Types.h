#pragma once

#include "include_all.h"

// Model params
#define MODEL2MODELBASE ModelBaseParams{params.programID,params.programSelectedID,params.name,params.show,params.drawMode}
#define BEZIER2MODELBASE ModelBaseParams{params.programID,params.programSelectedID,params.name,params.show,GL_LINE_STRIP}
#define BSPLINE2MODELBASE ModelBaseParams{params.programID,params.programSelectedID,params.name,params.show,GL_LINE_STRIP}
#define DISCRETECURVE2MODELBASE ModelBaseParams{params.programID,params.programSelectedID,params.name,params.show,GL_LINE_STRIP}
#define BEZIERSURFACE2MODELBASE ModelBaseParams{params.programID,params.programSelectedID,params.name,params.show,GL_TRIANGLES}

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

struct BezierParams {
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
    int modelIndex = 0;
    glm::ivec2 cursorPos = glm::ivec2(0, 0);
    glm::ivec2 windowSize = glm::ivec2(0, 0);
    glm::mat4 viewProj = glm::identity<glm::mat4>();
    bool selected = false;
    float selectionWidth = 1.f;
    glm::vec3 selectionColor = glm::vec3(1.f, 0, 0);
};

struct RenderShadowParams {
    float lineWidth = 1.f;
    glm::mat4 viewProj = glm::identity<glm::mat4>();
};

struct MeshRenderParams {
    float lineWidth;
    glm::vec3 cameraPos;
    std::vector<glm::vec4> lights;
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