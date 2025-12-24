#pragma once

#include "../include_all.h"

class ShadowMapController {
private:
    struct LightReference {
        GLuint* layerPtr;
        GLuint* matrixIndexPtr;
    };
    inline static std::vector<LightReference> handles2D;
    inline static std::vector<LightReference> handlesCube;

    // --- 2D Array (Spot & Directional) ---
    inline static GLuint texture2DArrayID = 0, fbo2DArrayID = 0;
    inline static int width2D = 1024, height2D = 1024, layers2D = 0;

    // --- CubeMap Array (Point Lights) ---
    inline static GLuint textureCubeArrayID = 0, fboCubeArrayID = 0;
    inline static int widthCube = 512, heightCube = 512, layersCube = 0; // layersCube = Count of lights (not multiplied by 6)
    inline static std::vector<GLuint*> handlesCube;

    // --- Indirection & Global State ---
    inline static GLint defViewport[4];
    inline static GLuint nextMatrixIndex = 0;
    inline static GLuint debugTextureID = 0;

    static void Allocate2DStorage() {
        if (texture2DArrayID != 0) glDeleteTextures(1, &texture2DArrayID);
        if (layers2D <= 0) return;

        glGenTextures(1, &texture2DArrayID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture2DArrayID);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, width2D, height2D, layers2D);

        SetupCommonTextureParams(GL_TEXTURE_2D_ARRAY);
    }

    static void AllocateCubeStorage() {
        if (textureCubeArrayID != 0) glDeleteTextures(1, &textureCubeArrayID);
        if (layersCube <= 0) return;

        glGenTextures(1, &textureCubeArrayID);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, textureCubeArrayID);
        // Depth parameter for CubeMap Array = number of layers * 6
        glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_DEPTH_COMPONENT32F, widthCube, heightCube, layersCube * 6);

        SetupCommonTextureParams(GL_TEXTURE_CUBE_MAP_ARRAY);
    }

    static void SetupCommonTextureParams(GLenum target) {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        if (target == GL_TEXTURE_CUBE_MAP_ARRAY) glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
    }

    static void RebuildMatrixIndices() {
        nextMatrixIndex = 0;

        // 1. Iterate 2D
        for (auto& h : handles2D) {
            *(h.matrixIndexPtr) = nextMatrixIndex;
            ++nextMatrixIndex;
        }

        // 2. Iterate Cube
        for (auto& h : handlesCube) {
            *(h.matrixIndexPtr) = nextMatrixIndex;
            nextMatrixIndex += 6;
        }
    }

public:
    static void Init(int w2d, int h2d, int wCube, int hCube) {
        width2D = w2d; height2D = h2d;
        widthCube = wCube; heightCube = hCube;

        glGenFramebuffers(1, &fbo2DArrayID);
        glGenFramebuffers(1, &fboCubeArrayID);

        nextMatrixIndex = 0;
    }

    static void Resize2D(int newWidth, int newHeight) {
        width2D = newWidth;
        height2D = newHeight;
        if (layers2D > 0) Allocate2DStorage();
    }

    static void ResizeCube(int newSize) {
        widthCube = newSize;
        heightCube = newSize;
        if (layersCube > 0) AllocateCubeStorage();
    }

    // --- Handling 2D layer (Spot/Dir) ---
    static GLuint Reserve2DLayer(GLuint*& lightLayerPtr, GLuint*& matrixIndexPtr) {
        GLuint layerIndex = (GLuint)handles2D.size();
        handles2D.push_back(LightReference{lightLayerPtr, matrixIndexPtr});
        *lightLayerPtr = layerIndex;

        // Handling matrix indirection (1 matrix/light)
        *matrixIndexPtr = nextMatrixIndex;
        ++nextMatrixIndex;

        layers2D = (int)handles2D.size();
        Allocate2DStorage();
        return layerIndex;
    }
    // --- Free 2D layer (Spot/Dir) ---
    static void Release2DLayer(GLuint index, GLuint* matrixIndexPtr) {
        if (index >= (GLuint)handles2D.size()) return;

        GLuint lastIndex = (GLuint)(handles2D.size() - 1);

        if (index != lastIndex) {
            // Swap-and-pop
            handles2D[index] = handles2D[lastIndex];
            // Update light source layer index
            if (handles2D[index].layerPtr) {
                *(handles2D[index].layerPtr) = index;
            }
        }

        handles2D.pop_back();
        layers2D = (int)handles2D.size();

        // Recalculate matrix indeces
        RebuildMatrixIndices();

        // Free VRAM
        Allocate2DStorage();
    }

    // --- Handling Cube layer (Point) ---
    static GLuint ReserveCubeLayer(GLuint*& lightLayerPtr, GLuint*& matrixIndexPtr) {
        GLuint layerIndex = (GLuint)handlesCube.size();
        handlesCube.push_back(LightReference{ lightLayerPtr, matrixIndexPtr });
        *lightLayerPtr = layerIndex;

        // Handling matrix indirection (6 matrix/light)
        *matrixIndexPtr = nextMatrixIndex;
        nextMatrixIndex += 6;

        layersCube = (int)handlesCube.size();
        AllocateCubeStorage();
        return layerIndex;
    }
    static void ReleaseCubeLayer(GLuint index, GLuint* matrixIndexPtr) {
        if (index >= (GLuint)handlesCube.size()) return;

        GLuint lastIndex = (GLuint)(handlesCube.size() - 1);

        if (index != lastIndex) {
            // Swap-and-pop
            handlesCube[index] = handlesCube[lastIndex];
            // Update light source layer index
            if (handlesCube[index].layerPtr) {
                *(handlesCube[index].layerPtr) = index;
            }
        }

        handlesCube.pop_back();
        layersCube = (int)handlesCube.size();

        // Recalculate matrix indeces
        RebuildMatrixIndices();

        // Free VRAM
        AllocateCubeStorage();
    }

    // --- Prepare for reading ---
    static void Bind2DForWriting(GLuint layerIndex, bool clear = true) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo2DArrayID);
        glGetIntegerv(GL_VIEWPORT, defViewport);
        glViewport(0, 0, width2D, height2D);

        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture2DArrayID, 0, layerIndex);
        if (clear) glClear(GL_DEPTH_BUFFER_BIT);
    }

    static void BindCubeFaceForWriting(GLuint cubeLayerIndex, GLuint faceIndex, bool clear = true) {
        glBindFramebuffer(GL_FRAMEBUFFER, fboCubeArrayID);
        glGetIntegerv(GL_VIEWPORT, defViewport);
        glViewport(0, 0, widthCube, heightCube);

        // Layer in the CubeMap Array: layer * 6 + face
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureCubeArrayID, 0, cubeLayerIndex * 6 + faceIndex);
        if (clear) glClear(GL_DEPTH_BUFFER_BIT);
    }

    // --- Prepare for reading ---
    static void BindAllForReading(GLuint startUnit2D, GLuint startUnitCube) {
        glActiveTexture(GL_TEXTURE0 + startUnit2D);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture2DArrayID);

        glActiveTexture(GL_TEXTURE0 + startUnitCube);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, textureCubeArrayID);
    }

    // --- Get textureView for ImGui ---
    static GLuint Get2DLayerTextureView(uint32_t layerIndex) {
        if (layerIndex >= (uint32_t)layers2D) return 0;

        glGenTextures(1, &debugTextureID);

        glTextureView(debugTextureID, GL_TEXTURE_2D, texture2DArrayID, GL_DEPTH_COMPONENT32F,
            0, 1, layerIndex, 1);

        glBindTexture(GL_TEXTURE_2D, debugTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

        return debugTextureID;
    }

    static GLuint GetCubeLayerTextureView(uint32_t cubeLayerIndex, uint32_t faceIndex) {
        if (cubeLayerIndex >= (uint32_t)layersCube || faceIndex > 5) return 0;

        glGenTextures(1, &debugTextureID);

        glTextureView(debugTextureID, GL_TEXTURE_2D, textureCubeArrayID, GL_DEPTH_COMPONENT32F,
            0, 1, cubeLayerIndex * 6 + faceIndex, 1);

        glBindTexture(GL_TEXTURE_2D, debugTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

        return debugTextureID;
    }

    static void Unbind() {
        glViewport(defViewport[0], defViewport[1], defViewport[2], defViewport[3]);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    static void Clean() {
        glDeleteFramebuffers(1, &fbo2DArrayID);
        fbo2DArrayID = 0;
        glDeleteFramebuffers(1, &fboCubeArrayID);
        fboCubeArrayID = 0;
        glDeleteTextures(1, &texture2DArrayID);
        texture2DArrayID = 0;
        glDeleteTextures(1, &textureCubeArrayID);
        textureCubeArrayID = 0;
        glDeleteTextures(1, &debugTextureID);
        debugTextureID = 0;
        handles2D.clear();
        handlesCube.clear();
        layers2D = 0;
        layersCube = 0;
    }
};