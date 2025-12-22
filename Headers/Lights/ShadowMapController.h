#pragma once

#include "../include_all.h"

class ShadowMapController {
private:
	inline static GLuint textureID = 0, fboID = 0;
	inline static int width = 0, height = 0, maxLayers = 0;
	inline static std::vector<GLuint*> handles;
	inline static GLint defViewport[4];

	static void AllocateStorage() {
		// Delete old texture if exists
		if (textureID != 0) {
			glDeleteTextures(1, &textureID);
			textureID = 0;
		}

		if (maxLayers <= 0) {
			return;
		}

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, width, height, maxLayers);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Framebuffer frissítése és ellenõrzése
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		// Hozzácsatoljuk az elsõ réteget, hogy érvényes legyen a státusz ellenõrzéskor
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::string errorMsg;
			switch (status) {
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
			case GL_FRAMEBUFFER_UNSUPPORTED:                   errorMsg = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
			default:                                           errorMsg = "Unknown Error (" + std::to_string(status) + ")"; break;
			}
			Log::errorToConsole("[ShadowMapController] Framebuffer status hiba: %s", errorMsg.c_str());
			Clean();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		Log::logToConsole("Frame buffer allocation complete for ", maxLayers, " layers with size ", width, "x", height);

		return;
	}

public:
	static void Init(int w, int h) {
		width = w;
		height = h;
		maxLayers = 0;
		glGenFramebuffers(1, &fboID);
		glGenTextures(1, &textureID);
	}
	static void Clean() {
		glDeleteFramebuffers(1, &fboID);
		fboID = 0;
		glDeleteTextures(1, &textureID);
		textureID = 0;
		handles.clear();
		maxLayers = 0;
	}
	static void Resize(int newWidth, int newHeight) {
		width = newWidth;
		height = newHeight;
		if (maxLayers > 0) AllocateStorage();
	}

	static GLuint ReserveLayer(GLuint* lightIndexPtr) {
		++maxLayers;
		handles.push_back({ lightIndexPtr });

		AllocateStorage();

		GLuint newIndex = (GLuint)(maxLayers - 1);
		*lightIndexPtr = newIndex;
		return newIndex;
	}
	static void ReleaseLayer(GLuint index) {
		if (index >= (GLuint)handles.size()) return;

		GLuint lastIndex = (GLuint)(maxLayers - 1);

		if (index != lastIndex) {
			// Swap-and-pop: az utolsó elemet az eltávolított helyére rakjuk
			handles[index] = handles[lastIndex];

			// Frissítjük a költöztetett fényforrás indexét
			if (handles[index]) {
				*(handles[index]) = index;
			}
		}

		handles.pop_back();
		--maxLayers;

		// Mindig újrafoglalunk, hogy a VRAM felszabaduljon
		AllocateStorage();
	}

	static void BindForWriting(GLuint layerIndex, bool clear = true) {
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);

		glGetIntegerv(GL_VIEWPORT, defViewport);
		glViewport(0, 0, width, height);

		// DEBUG: Check if the FBO is complete
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "FBO Incomplete! Status: " << status << " Layer: " << layerIndex << std::endl;
			exit(1);
		}

		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0, layerIndex);
		if (clear) {
			glClear(GL_DEPTH_BUFFER_BIT);
		}
	}
	static void BindForReading(GLuint textureUnit) {
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	}
	static void Unbind() {
		glViewport(defViewport[0], defViewport[1], defViewport[2], defViewport[3]);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	static GLuint GetLayerTextureID(uint32_t layerIndex) {
		if (layerIndex >= maxLayers) return 0;

		GLuint viewID;
		// Létrehozunk egy új textúra nevet (handle)
		glGenTextures(1, &viewID);

		// Létrehozunk egy nézetet, ami az Array egyetlen rétegét sima 2D textúrának mutatja
		// Paraméterek: új ID, cél, forrás ID, belsõ formátum, 
		// kezdõ mipmap, szintek száma, kezdõ réteg, rétegek száma
		glTextureView(viewID, GL_TEXTURE_2D, textureID, GL_DEPTH_COMPONENT24,
			0, 1, layerIndex, 1);

		// Fontos: a Texture View paramétereit (filterek) külön be kell állítani a nézeten is
		glBindTexture(GL_TEXTURE_2D, viewID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// Depth textúra megjelenítéséhez ImGui-ban néha szükség lehet swizzle-re, 
		// hogy a Z érték minden csatornába (RGB) bekerüljön:
		GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

		return viewID;
	}
};