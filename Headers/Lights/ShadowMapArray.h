#include "../include_all.h"

class ShadowMapArray {
private:
    inline static GLuint textureID = 0;
    inline static GLuint fboID = 0;
    inline static int width = 0, height = 0, maxLayers = 0;
    inline static std::vector<bool> usedLayers;

public:
    static void init(int w, int h, int layers) {
        width = w; height = h; maxLayers = layers;
        usedLayers.assign(maxLayers, false);

        glGenFramebuffers(1, &fboID);
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

        // Mélységi textúra tömb foglalása
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT,
            width, height, maxLayers,
            0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, fboID);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    static int reserveLayer() {
        for (int i = 0; i < maxLayers; ++i) {
            if (!usedLayers[i]) {
                usedLayers[i] = true;
                return i;
            }
        }
        return -1;
    }

    static void releaseLayer(int index) {
        if (index >= 0 && index < maxLayers) usedLayers[index] = false;
    }

    static void bindForWriting(int layerIndex) {
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);
        glViewport(0, 0, width, height);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0, layerIndex);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    static void bindForReading(GLuint textureUnit) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    }

    static GLuint getTextureID() { return textureID; }
};