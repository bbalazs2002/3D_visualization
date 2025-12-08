#pragma once

#include "include_all.h"

class CMyApp
{
public:

	inline static int MeshID = -1;

	CMyApp() {}
	~CMyApp() {}

	bool Init();
	void Clean();

	void Update(const SUpdateInfo&);
	void Render() const;
	void RenderGUI();

	void KeyboardDown(const SDL_KeyboardEvent&);
	void KeyboardUp(const SDL_KeyboardEvent&);
	void MouseMove(const SDL_MouseMotionEvent&);
	void MouseDown(const SDL_MouseButtonEvent&);
	void MouseUp(const SDL_MouseButtonEvent&);
	void MouseWheel(const SDL_MouseWheelEvent&);
	void Resize(int, int);

	void OtherEvent(const SDL_Event&);
protected:
	void SetupDebugCallback();

	//
	// Variables
	//
	int m_width = 0, m_height = 0;
	glm::ivec2 m_cursorPos = glm::ivec2(0, 0);
	float m_ElapsedTimeInSec = 0.0f;
	int m_selectedModel = -1;
	int m_selectedLight = -1;
	float m_selectionWidth = 2.f;
	float m_lineWidth = 1.f;
	bool m_cursorMoved = false;

	bool m_showAxes = true;
	bool m_renderShadows = true;
	int m_shadowBufferSize = 1024;

	std::vector<glm::vec4> m_newIntpolPoints{};
	std::vector<float> m_newTParams{};

	glm::vec3 m_selColor{ 1, 0, 0 };

	// Camera
	Camera m_camera;
	CameraManipulator m_cameraManipulator;

	// Shader program variables
	GLuint m_programAxesID = 0;						// Program showing X,Y,Z directions
	GLuint m_programSkyboxID = 0;					// Skybox shaders
	GLuint m_programModelID = 0;					// Drawing models
	GLuint m_programSelectedID = 0;					// Draw selection onto model
	GLuint m_programBezierID = 0;					// Draw Bezier-curves
	GLuint m_programBezierSelectedID = 0;			// Draw Bezier-curve selection
	GLuint m_programDiscreteCurveID = 0;			// Draw Discrete curves
	GLuint m_programDiscreteCurveSelectedID = 0;	// Draw Discrete curve selection
	GLuint m_programBSplineID = 0;					// Draw BSpline-curves
	GLuint m_programBSplineSelectedID = 0;			// Draw BSpline-curve selection
	GLuint m_programBezierSurfaceID = 0;			// Draw Bezier-surface
	GLuint m_programBezierSurfaceSelectedID = 0;	// Draw Bezier-surface selection
	GLuint m_programShadowID = 0;					// Render shadow texture

	GLuint m_programDirectionLightID = 0;			// Render direction light selection
	GLuint m_programPointLightID = 0;				// Render point light selection
	GLuint m_programSpotLightID = 0;				// Render spot light selection

	// Shader initialization and termination
	void InitShaders();
	void CleanShaders();
	void InitSkyboxShader();
	void CleanSkyboxShader();
	void InitAxesShader();
	void CleanAxesShader();

	// Geometry variables
	OGLObject m_SkyboxGPU = {};
	std::vector<ModelBase*> m_models{};

	// Geometry initialization and termination
	void InitGeometry();
	void CleanGeometry();
	void InitSkyboxGeometry();
	void CleanSkyboxGeometry();
	void InitModels();
	void CleanModels();

	// Textures
	GLuint m_skyboxTextureID = 0;
	GLuint m_modelTextureID = 0;
	GLuint m_shadowTextureID = 0;

	// Lighting
	void InitLights();
	void CleanLights();
	std::vector<Light*> m_lights{};

	// Texture initialization
	void InitTexture();
	void CleanTexture();
	void InitSkyboxTexture();
	void CleanSkyboxTexture();
	void InitResolutionDependentResources(glm::vec2 bufferSize);
	void CleanResolutionDependentResources();

	// Buffer IDs
	GLuint m_ModelIDBufferID = 0;
	GLuint m_LightsBufferID = 0;
	GLuint m_FBOShadowID = 0;

	// Buffer initialization
	void InitBuffers();
	void InitLightBuffer();
	void CleanBuffers();

	// rendering methods
	void DrawAxes() const;
	void RenderModels() const;
	void RenderLightSuorce() const;
	void RenderSkybox() const;
};