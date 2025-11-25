#include "../Headers/include_all.h"

void CMyApp::SetupDebugCallback()
{
	// Enable and set the debug callback function if we are in debug context
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void CMyApp::InitShaders()
{
	// Drawing models
	m_programModelID = glCreateProgram();
	ProgramBuilder{ m_programModelID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/Models/Vert_Model.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/Models/Frag_Lighting.frag")
		.Link();

	m_programSelectedID = glCreateProgram();
	ProgramBuilder{ m_programSelectedID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/Models/Vert_Selected.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/Models/Frag_Selected.frag")
		.Link();

	// Bezier
	m_programBezierID = glCreateProgram();
	ProgramBuilder{ m_programBezierID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/Bezier/Vert_Bezier.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/Bezier/Frag_Bezier.frag")
		.Link();

	m_programBezierSelectedID = glCreateProgram();
	ProgramBuilder{ m_programBezierSelectedID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/Bezier/Vert_BezierSelected.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/Bezier/Frag_Bezier.frag")
		.Link();

	// B-Spline
	m_programBSplineID = glCreateProgram();
	ProgramBuilder{ m_programBSplineID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/BSpline/Vert_BSpline.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/BSpline/Frag_BSpline.frag")
		.Link();

	m_programBSplineSelectedID = glCreateProgram();
	ProgramBuilder{ m_programBSplineSelectedID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/BSpline/Vert_BSplineSelected.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/BSpline/Frag_BSpline.frag")
		.Link();

	// DiscreteCurve
	m_programDiscreteCurveID = glCreateProgram();
	ProgramBuilder{ m_programDiscreteCurveID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/DiscreteCurve/Vert_DiscreteCurve.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/DiscreteCurve/Frag_DiscreteCurve.frag")
		.Link();

	m_programDiscreteCurveSelectedID = glCreateProgram();
	ProgramBuilder{ m_programDiscreteCurveSelectedID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/DiscreteCurve/Vert_DiscreteCurveSelected.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/DiscreteCurve/Frag_DiscreteCurve.frag")
		.Link();

	// Bezier-surface
	m_programBezierSurfaceID = glCreateProgram();
	ProgramBuilder{ m_programBezierSurfaceID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/BezierSurface/Vert_BezierSurface.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/BezierSurface/Frag_BezierSurface.frag")
		.Link();

	m_programBezierSurfaceSelectedID = glCreateProgram();
	ProgramBuilder{ m_programBezierSurfaceSelectedID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/BezierSurface/Vert_BezierSurfaceSelected.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/BezierSurface/Frag_BezierSurfaceSelected.frag")
		.Link();

	// Shadows
	/*
	m_programShadowID = glCreateProgram();
	ProgramBuilder{ m_programShadowID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/Shadow/vert_Shadow.vert")
		.Link();
	*/

	InitAxesShader();
	InitSkyboxShader();
}

void CMyApp::CleanShaders()
{
	glDeleteProgram(m_programModelID);
	m_programModelID = 0;
	glDeleteProgram(m_programSelectedID);
	m_programSelectedID = 0;

	glDeleteProgram(m_programBezierID);
	m_programBezierID = 0;
	glDeleteProgram(m_programBezierSelectedID);
	m_programBezierSelectedID = 0;
	
	glDeleteProgram(m_programBSplineID);
	m_programBSplineID = 0;
	glDeleteProgram(m_programBSplineSelectedID);
	m_programBSplineSelectedID = 0;

	glDeleteProgram(m_programDiscreteCurveID);
	m_programDiscreteCurveID = 0;
	glDeleteProgram(m_programDiscreteCurveSelectedID);
	m_programDiscreteCurveSelectedID = 0;

	glDeleteProgram(m_programBezierSurfaceID);
	m_programBezierSurfaceID = 0;
	glDeleteProgram(m_programBezierSurfaceSelectedID);
	m_programBezierSurfaceSelectedID = 0;

	glDeleteProgram(m_programShadowID);
	m_programShadowID = 0;

	CleanSkyboxShader();
	CleanAxesShader();
}

void CMyApp::InitSkyboxShader() {
	m_programSkyboxID = glCreateProgram();
	ProgramBuilder{ m_programSkyboxID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/Skybox/Vert_skybox.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/Skybox/Frag_skybox_skeleton.frag")
		.Link();
}

void CMyApp::CleanSkyboxShader() {
	glDeleteProgram(m_programSkyboxID);
	m_programSkyboxID = 0;
}

void CMyApp::InitAxesShader()
{
	m_programAxesID = glCreateProgram();
	ProgramBuilder{ m_programAxesID }
		.ShaderStage(GL_VERTEX_SHADER, "Shaders/Axes/Vert_axes.vert")
		.ShaderStage(GL_FRAGMENT_SHADER, "Shaders/Axes/Frag_PosCol.frag")
		.Link();
}

void CMyApp::CleanAxesShader()
{
	glDeleteProgram(m_programAxesID);
	m_programAxesID = 0;
}

void CMyApp::InitGeometry()
{
	InitModels();
	InitSkyboxGeometry();
}

void CMyApp::CleanGeometry()
{
	CleanModels();
	CleanSkyboxGeometry();
}

void CMyApp::InitSkyboxGeometry() {
	// skybox geo
	MeshObject<glm::vec3> skyboxCPU =
	{
		std::vector<glm::vec3>
		{
		// back
		glm::vec3(-1, -1, -1),
		glm::vec3(1, -1, -1),
		glm::vec3(1,  1, -1),
		glm::vec3(-1,  1, -1),
			// front
			glm::vec3(-1, -1, 1),
			glm::vec3(1, -1, 1),
			glm::vec3(1,  1, 1),
			glm::vec3(-1,  1, 1),
		},

		std::vector<GLuint>
		{
		// back
		0, 1, 2,
		2, 3, 0,
			// front
			4, 6, 5,
			6, 4, 7,
			// left
			0, 3, 4,
			4, 3, 7,
			// right
			1, 5, 2,
			5, 6, 2,
			// bottom
			1, 0, 4,
			1, 4, 5,
			// top
			3, 2, 6,
			3, 6, 7,
		}
	};

	m_SkyboxGPU = CreateGLObjectFromMesh(skyboxCPU, { { 0, offsetof(glm::vec3, x), 3, GL_FLOAT } });
}

void CMyApp::CleanSkyboxGeometry()
{
	CleanOGLObject(m_SkyboxGPU);
}

void CMyApp::InitModels() {
	{
		// Bezier-surface
		/*
		m_models.push_back(new BezierSurface(
			BezierSurfaceParams{
				m_programBezierSurfaceID,
				m_programBezierSurfaceSelectedID,
				glm::vec2{10, 10},
				"Bezier-surface",
				true, false
			}
		));
		((BezierSurface*)m_models[m_models.size() - 1])->SetCtrlPoints(glm::vec2{6, 5}, std::vector<glm::vec4>{
				glm::vec4{ -2,2,-2,1 }, glm::vec4{ -1,0,-2,1 }, glm::vec4{ 0,0,-2,1 }, glm::vec4{ 1,0,-2,1 }, glm::vec4{ 2,0,-2,1 },
				glm::vec4{ -2,0,-1,1 }, glm::vec4{ -1,2,-1,1 }, glm::vec4{ 0,0,-1,1 }, glm::vec4{ 1,0,-1,1 }, glm::vec4{ 2,0,-1,1 },
				glm::vec4{ -2,0,0,1 }, glm::vec4{ -1,0,0,1 }, glm::vec4{ 0,10,0,1 }, glm::vec4{ 1,0,0,1 }, glm::vec4{ 2,0,0,1 },
				glm::vec4{ -2,0,1,1 }, glm::vec4{ -1,0,1,1 }, glm::vec4{ 0,0,1,1 }, glm::vec4{ 1,2,1,1 }, glm::vec4{ 2,0,1,1 },
				glm::vec4{ -2,0,2,1 }, glm::vec4{ -1,0,2,1 }, glm::vec4{ 0,0,2,1 }, glm::vec4{ 1,0,2,1 }, glm::vec4{ 2,2,2,1 },
				glm::vec4{ -2,0,3,1 }, glm::vec4{ -1,0,3,1 }, glm::vec4{ 0,0,3,1 }, glm::vec4{ 1,0,3,1 }, glm::vec4{ 2,2,3,1 }
		});
		((BezierSurface*)m_models[m_models.size() - 1])->SetMaterial(new Material{
			"Bezier-surface-material",
			glm::vec3(1.f), glm::vec3(1.f), glm::vec3(1.f),
			32.f,
			m_modelTextureID, 0, 0, 0
		});
		*/

		// Bezier-surface
		/*
		m_models.push_back(new BezierSurface(
			BezierSurfaceParams{
				m_programBezierSurfaceID,
				m_programBezierSurfaceSelectedID,
				glm::vec2{10, 10},
				"Bezier-surface-2",
				true, false
			}
		));
		((BezierSurface*)m_models[m_models.size() - 1])->SetCtrlPoints(glm::vec2{ 3, 3 }, std::vector<glm::vec4>{
			glm::vec4{ -2,0,-2,1 }, glm::vec4{ -1,0,-2,1 }, glm::vec4{ 0,0,-2,1 },
			glm::vec4{ -2,0,-1,1 }, glm::vec4{ -1,5,-1,1 }, glm::vec4{ 0,0,-1,1 },
			glm::vec4{ -2,0,0,1 }, glm::vec4{ -1,0,0,1 }, glm::vec4{ 0,0,0,1 }
		});
		((BezierSurface*)m_models[m_models.size() - 1])->SetMaterial(new Material{
			"Bezier-surface-material",
			glm::vec3(1.f), glm::vec3(1.f), glm::vec3(1.f),
			32.f,
			m_modelTextureID, 0, 0, 0
			});
		*/

		// Bezier
		/*
		m_models.push_back(new Bezier(
			BezierParams{
				m_programBezierID,
				m_programBezierSelectedID,
				10,
				"Bezier",
				true
			}
		));
		((Bezier*)m_models[m_models.size() - 1])->SetCtrlPoints(std::vector<glm::vec4>{
			glm::vec4{ 0,0,0,1 },
			glm::vec4{ 5,0,0,1 },
			glm::vec4{ 0,5,0,1 },
			glm::vec4{ 0,0,5,1 },
		});
		*/

		// Cube
		/*
		m_models.push_back(new Model(
			ModelParams{
				m_programModelID,
				m_programSelectedID,
				"Cube",
				false
			}
		));
		m_models[m_models.size() - 1]->AddTransform(glm::transpose(glm::mat4{
				{ 1, 0, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 1, 0 },
				{ 0, 0, 0, 1 }
			}
		));
		((Model*)m_models[m_models.size() - 1])->SetObjPath("C:\\Users\\Balazs\\Documents\\ELTE\\2025-26-01\\geommod\\Transforms\\Assets\\cube.obj");
		*/

		// Equinox
		m_models.push_back(new Model(
			ModelParams{
				m_programModelID,
				m_programSelectedID,
				"Equinox",
				false
			}
		));
		m_models[m_models.size() - 1]->AddTransform(glm::transpose(glm::mat4{
				{ 1, 0, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 1, 0 },
				{ 0, 0, 0, 1 }
			}
		));
		((Model*) m_models[m_models.size() - 1])->SetObjPath("C:\\Users\\Balazs\\Documents\\ELTE\\2025-26-01\\geommod\\3D_visualization\\Assets\\Equinox-render\\Equinox.obj");

		// B-Spline
		/*
		m_models.push_back(new BSpline(
			BSplineParams{
				m_programBSplineID,
				m_programBSplineSelectedID,
				10,
				"B-Spline",
				true
			}
		));
		((BSpline*)m_models[m_models.size() - 1])->SetCtrlPoints(std::vector<glm::vec4>{
			glm::vec4{ 10.0, 0.0, 0.0, 1 },
			glm::vec4{ 5.0, 5.0, 1.0, 1 },
			glm::vec4{ 0.0, 10.0, 2.0, 1 },
			glm::vec4{ -5.0, 5.0, 3.0, 1 },
			glm::vec4{ -10.0, 0.0, 4.0, 1 },
			glm::vec4{ -5.0, -5.0, 5.0, 1 },
			glm::vec4{ 0.0, -10.0, 6.0, 1 },
			glm::vec4{ 5.0, -5.0, 7.0, 1 }
		});
		((BSpline*)m_models[m_models.size() - 1])->SetKnots(std::vector<float>{0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2});
		*/

		// Semi circle interpolation
		/*
		BSpline* semicircle = BSpline::CreateInterpolated(std::vector<glm::vec4>{
			glm::vec4(10.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(7.07f, 7.07f, 0.0f, 1.0f),
			glm::vec4(0.0f, 10.0f, 0.0f, 1.0f),
			glm::vec4(-7.07f, 7.07f, 0.0f, 1.0f),
			glm::vec4(-10.0f, 0.0f, 0.0f, 1.0f)
		}, BSplineParams{
			m_programBSplineID,
			m_programBSplineSelectedID,
			10,
			"B-Spline-2",
			false
		});
		if (semicircle != nullptr) {
			semicircle->SetColor(glm::vec3(0, 1, 1));
			m_models.push_back(semicircle);
		}
		else {
			Log::errorToConsole("Failed to interpolate points");
		}
		*/

		// Spiral interpolation
		/*
		BSpline* spiral = BSpline::CreateInterpolated(std::vector<glm::vec4>{
			glm::vec4(5.0f, 0.0f, 0.0f, 1.0f),  // Q0
			glm::vec4(0.0f, 5.0f, 1.f, 1.0f),   // Q1
			glm::vec4(-5.0f, 0.0f, 2.f, 1.0f),  // Q2
			glm::vec4(0.0f, -5.0f, 3.f, 1.0f),  // Q3
			glm::vec4(5.0f, 0.0f, 4.f, 1.0f),   // Q4
			glm::vec4(0.0f, 5.0f, 5.f, 1.0f),   // Q5
			glm::vec4(-5.0f, 0.0f, 6.f, 1.0f)   // Q6
		},
			BSplineParams{
			m_programBSplineID,
			m_programBSplineSelectedID,
			10,
			"B-Spline-3",
			false
		});
		if (spiral != nullptr) {
			m_models.push_back(spiral);
		}
		else {
			Log::errorToConsole("Failed to interpolate points");
		}
		*/

		// Discrete Curve
		/*
		m_models.push_back(new DiscreteCurve(
			DiscreteCurveParams{
				m_programDiscreteCurveID,
				m_programDiscreteCurveSelectedID,
				"DiscreteCurve",
				true
			}
		));
		((DiscreteCurve*)m_models[m_models.size() - 1])->SetCtrlPoints(std::vector<glm::vec4>{
			glm::vec4{ 10.0, 0.0, 0.0, 1 },
			glm::vec4{ 5.0, 5.0, 1.0, 1 },
			glm::vec4{ 0.0, 10.0, 2.0, 1 },
			glm::vec4{ -5.0, 5.0, 3.0, 1 },
			glm::vec4{ -10.0, 0.0, 4.0, 1 },
			glm::vec4{ -5.0, -5.0, 5.0, 1 },
			glm::vec4{ 0.0, -10.0, 6.0, 1 },
			glm::vec4{ 5.0, -5.0, 7.0, 1 }
		});
		*/
	}
}

void CMyApp::CleanModels() {
	for (int i = 0; i < m_models.size(); ++i) {
		delete(m_models[i]);
	}
	m_models.clear();
}

void CMyApp::InitTexture() {
	// Model texture
	{
		ImageRGBA image;
		image = ImageFromFile("Assets/metal.png");
		glGenTextures(1, &m_modelTextureID);
		glBindTexture(GL_TEXTURE_2D, m_modelTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	InitSkyboxTexture();

	glBindTexture(GL_TEXTURE_2D, 0);
}

void CMyApp::CleanTexture() {
	glDeleteTextures(1, &m_modelTextureID);
	CleanSkyboxTexture();
}

void CMyApp::InitSkyboxTexture() {
	// skybox texture
	static const char* skyboxFiles[6] = {
		"Assets/xpos.png",
		"Assets/xneg.png",
		"Assets/ypos.png",
		"Assets/yneg.png",
		"Assets/zpos.png",
		"Assets/zneg.png",
	};

	ImageRGBA images[6];
	for (int i = 0; i < 6; ++i)
	{
		images[i] = ImageFromFile(skyboxFiles[i], false);
	}

	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_skyboxTextureID);
	glTextureStorage2D(m_skyboxTextureID, 1, GL_RGBA8, images[0].width, images[0].height);

	for (int face = 0; face < 6; ++face)
	{
		glTextureSubImage3D(m_skyboxTextureID, 0, 0, 0, face, images[face].width, images[face].height, 1, GL_RGBA, GL_UNSIGNED_BYTE, images[face].data());
	}

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void CMyApp::CleanSkyboxTexture() {
	glDeleteTextures(1, &m_skyboxTextureID);
	m_skyboxTextureID = 0;
}

void CMyApp::InitBuffers() {
	// SSBO for model below cursor
	glGenBuffers(1, &m_ModelIDBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ModelIDBufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 2, nullptr, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ModelIDBufferID);

	// framebuffer for shadow texture
	// glCreateFramebuffers(1, &m_FBOShadowID);
}
void CMyApp::CleanBuffers() {
	glDeleteBuffers(1, &m_ModelIDBufferID);
	m_ModelIDBufferID = 0;

	// glDeleteFramebuffers(1, &m_FBOShadowID);
	// m_FBOShadowID = 0;
}

void CMyApp::InitResolutionDependentResources(glm::vec2 bufferSize) {
	return;
	// We use texture instead of renderbuffer,
	// because we will sample it in the shader	

	glCreateTextures(GL_TEXTURE_2D, 1, &m_shadowTextureID);
	glTextureStorage2D(m_shadowTextureID, 1, GL_DEPTH_COMPONENT24, bufferSize.x, bufferSize.y);

	glNamedFramebufferTexture(m_FBOShadowID, GL_DEPTH_ATTACHMENT, m_shadowTextureID, 0);

	// Completeness check
	GLenum status = glCheckNamedFramebufferStatus(m_FBOShadowID, GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (status) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[InitFramebuffer] Incomplete framebuffer GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT!");
			Log::errorToConsole("[InitFramebuffer] Incomplete framebuffer GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT!");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[InitFramebuffer] Incomplete framebuffer GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT!");
			Log::errorToConsole("[InitFramebuffer] Incomplete framebuffer GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT!");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[InitFramebuffer] Incomplete framebuffer GL_FRAMEBUFFER_UNSUPPORTED!");
			Log::errorToConsole("[InitFramebuffer] Incomplete framebuffer GL_FRAMEBUFFER_UNSUPPORTED!");
			break;
		}
	}
}

void CMyApp::CleanResolutionDependentResources()
{
	glDeleteTextures(1, &m_shadowTextureID);
	m_shadowTextureID = 0;
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	// Set a bluish clear color
	// glClear() will use this for clearing the color buffer.
	// glClearColor(0.125f, 0.25f, 0.5f, 1.0f);
	glClearColor(0, 0, 0, 1.0f);

	InitShaders();
	InitTexture();
	InitGeometry();
	InitBuffers();
	// InitResolutionDependentResources(glm::vec2(m_shadowBufferSize));

	//
	// Other
	//
	glEnable(GL_CULL_FACE);	 // Enable discarding the back-facing faces.
	glCullFace(GL_BACK);     // GL_BACK: facets facing away from camera, GL_FRONT: facets facing towards the camera
	glEnable(GL_DEPTH_TEST); // Enable depth testing. (for overlapping geometry)
	glDepthFunc(GL_LESS);

	// Camera
	m_camera.SetView(
		glm::vec3(0, 20, 20),	// From where we look at the scene - eye
		glm::vec3(0, 4, 0),		// Which point of the scene we are looking at - at
		glm::vec3(0, 1, 0)		// Upwards direction - up
	);
	m_cameraManipulator.SetCamera(&m_camera);

	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
	CleanTexture();
	CleanBuffers();
	CleanResolutionDependentResources();
}

void CMyApp::Update(const SUpdateInfo& updateInfo)
{
	m_cameraManipulator.Update(updateInfo.DeltaTimeInSec);
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;
}

void CMyApp::DrawAxes() const
{
	glUseProgram(m_programAxesID);

	glm::mat4 axisWorld = glm::translate(m_camera.GetAt());
	glProgramUniformMatrix4fv(m_programAxesID, ul(m_programAxesID, "viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));
	glProgramUniformMatrix4fv(m_programAxesID, ul(m_programAxesID, "world"), 1, GL_FALSE, glm::value_ptr(axisWorld));

	// We always want to see it, regardless of whether there is an object in front of it
	glDisable(GL_DEPTH_TEST);

	glDrawArrays(GL_LINES, 0, 6);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
}

void CMyApp::RenderModels() const {

	// update first vec4 in the buffer to the default model id
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ModelIDBufferID);
	glm::vec4 defObjID = glm::vec4(-1.f);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4), &defObjID);

	int objCount = 0;
	glm::vec2 cursorPos = glm::vec2(m_cursorPos.x, m_cursorPos.y);
	for (auto m : m_models) {
		RenderParams rp{
			m_lineWidth, m_camera.GetEye(), std::vector<glm::vec4>{m_lightPos},
			objCount, cursorPos, glm::vec2(m_width, m_height),
			m_camera.GetViewProj(), (m_selectedModel == objCount),
			m_selectionWidth, glm::vec3(m_selColor[0], m_selColor[1], m_selColor[2])
		};
		m->Render(&rp);
		++objCount;
	}
}

void CMyApp::RenderSkybox() const {
	glUseProgram(m_programSkyboxID);

	glProgramUniform1i(m_programSkyboxID, ul(m_programSkyboxID, "skyboxTexture"), 1);
	glProgramUniformMatrix4fv(m_programSkyboxID, ul(m_programSkyboxID, "viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));
	glProgramUniformMatrix4fv(m_programSkyboxID, ul(m_programSkyboxID, "world"), 1, GL_FALSE, glm::value_ptr(glm::translate(m_camera.GetEye())));

	// Save the last Z-test, namely the relation by which we update the pixel.
	GLint prevDepthFnc;
	glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFnc);

	// Now we use less-then-or-equal, because we push everything to the far clipping plane
	glDepthFunc(GL_LEQUAL);

	glBindTextureUnit(1, m_skyboxTextureID);
	glBindVertexArray(m_SkyboxGPU.vaoID);

	glDrawElements(GL_TRIANGLES, m_SkyboxGPU.count, GL_UNSIGNED_INT, nullptr);

	glDepthFunc(prevDepthFnc);

	glUseProgram(0);
	glBindVertexArray(0);
	glBindTextureUnit(0, 0);
}

void CMyApp::Render() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderModels();
	if (m_showAxes) {
		DrawAxes();
	}
	// RenderSkybox();

	// exit(1);

}

void CMyApp::RenderGUI()
{
	// OBJECT OPTIONS WINDOW
	if (m_selectedModel >= 0 && m_selectedModel < m_models.size()) {

		ModelBase* m = m_models[m_selectedModel];

		// Delete selected model if marked
		if (m->MarkedForDeletion()) {
			delete(m);
			m_models.erase(m_models.begin() + m_selectedModel);
			m_selectedModel = -1;
			return;
		}

		if (ImGui::Begin("Object editor")) {
			// Render type specific options
			m->RenderGUI(&m_models);

			// Render general options
			m->RenderGUIBase();
		}
		ImGui::End();
	}

	// GLOBAL OPTIONS WINDOW
	if (ImGui::Begin("Options window")) {
		ImGui::Text("Render resolution %dx%d", m_width, m_height);
		std::stringstream cursorPos;
		cursorPos << "Cursor position: " << m_cursorPos[0] << "; " << m_cursorPos[1];
		ImGui::Text(cursorPos.str().c_str());
		ImGui::Checkbox("Show axes", &m_showAxes);
		ImGui::SliderFloat3("Light position", &m_lightPos[0], -10.f, 10.f);
		ImGui::InputInt("Shown mesh", &CMyApp::MeshID);
		ImGui::InputInt("Selected model", &m_selectedModel);
		if (ImGui::BeginCombo("Select model", (m_selectedModel >= 0 && m_selectedModel < m_models.size()) ? m_models[m_selectedModel]->GetName().c_str() : ""))
		{
			for (int i = 0; i < m_models.size(); ++i) {
				if (ImGui::Selectable(m_models[i]->GetName().c_str(), m_selectedModel == i)) {
					m_selectedModel = i;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::SliderFloat("Selection width", &m_selectionWidth, 1.f, 10.f);
		ImGui::ColorEdit3("Selection color", &m_selColor.r);
		ImGui::SliderFloat("Line width", &m_lineWidth, 1.f, 10.f);
		static int bufferResolutionLevel = 10;
		std::string bufferResolutionText = std::to_string(1 << bufferResolutionLevel);
		if (ImGui::SliderInt("Shadow resolution level", &bufferResolutionLevel, 5, 12, bufferResolutionText.c_str()))
		{
			m_shadowBufferSize = 1 << bufferResolutionLevel;
			CleanResolutionDependentResources();
			InitResolutionDependentResources(glm::vec2(m_shadowBufferSize));
		}

		// Add new model
		if (ImGui::Button("Add model")) {
			m_models.push_back(new Model(
				ModelParams{
					m_programModelID,
					m_programSelectedID
				}
			));
		}
		// Add new curve
		if (ImGui::Button("Add Bezier-curve")) {
			m_models.push_back(new Bezier(
				BezierParams{
					m_programBezierID,
					m_programBezierSelectedID
				}
			));
		}
		ImGui::SameLine();
		if (ImGui::Button("Add B-Spline")) {
			m_models.push_back(new BSpline(
				BSplineParams{
					m_programBSplineID,
					m_programBSplineSelectedID
				}
			));
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Discrete-curve")) {
			m_models.push_back(new DiscreteCurve(
				DiscreteCurveParams{
					m_programDiscreteCurveID,
					m_programDiscreteCurveSelectedID
				}
			));
		}
		// Add new surface
		if (ImGui::Button("Add Bezier-surface")) {
			m_models.push_back(new BezierSurface(
				BezierSurfaceParams{
					m_programBezierSurfaceID,
					m_programBezierSurfaceSelectedID
				}
			));
		}

		// Log data to console
		if (ImGui::Button("Log data to console")) {
			std::cout << "Scene data ----------------------------------" << std::endl << std::endl;
			for (auto m : m_models) {
				std::cout << m->toString() << std::endl;
			}
			std::cout << "---------------------------------------------" << std::endl;
		}
	}
	ImGui::End();

	// INTERPOLATION WINDOW
	/*
	if (ImGui::Begin("Cubic B-Spline interpolation window")) {
		// Points
		if (ImGui::CollapsingHeader("Points")) {
			int intPointCount = 0;
			for (auto p : m_newIntpolPoints) {
				glm::vec4 point = p;
				std::stringstream label;
				label << "Point " << intPointCount;
				if (ImGui::InputFloat3(label.str().c_str(), &point.x)) {
					m_newIntpolPoints[intPointCount] = point;
				}
				ImGui::SameLine();
				label.str("");
				label << "Delete #" << intPointCount;
				if (ImGui::Button(label.str().c_str())) {
					m_newIntpolPoints.erase(m_newIntpolPoints.begin() + intPointCount);
					break;
				}
				++intPointCount;
			}
			ImGui::Spacing();
			if (ImGui::Button("Add point")) {
				m_newIntpolPoints.push_back(glm::vec4(0,0,0,1));
			}
		}

		// Parameters
		if (ImGui::CollapsingHeader("Params")) {
			int pCount = 0;
			for (auto k : m_newTParams) {
				float knot = k;
				std::stringstream label;
				label << "t #" << pCount;
				if (ImGui::InputFloat(label.str().c_str(), &knot)) {
					m_newTParams[pCount] = knot;
				}
				ImGui::SameLine();
				label.str("");
				label << "Delete #" << pCount;
				if (ImGui::Button(label.str().c_str())) {
					m_newTParams.erase(m_newTParams.begin() + pCount);
					break;
				}
				++pCount;
			}
			ImGui::Spacing();
			if (ImGui::Button("Add param")) {
				m_newTParams.push_back(0);
			}
		}

		ImGui::Spacing();

		if (ImGui::Button("Interpolate")) {
			BSpline* bs = BSpline::CreateInterpolatedWithParams(
				m_newIntpolPoints, m_newTParams,
				BSplineParams{
					m_programBSplineID,
					m_programBSplineSelectedID,
					10,
					std::vector<float>{},
					"",
					true
				}
			);
			if (bs != nullptr) {
				m_models.push_back(bs);
			}
			else {
				Log::errorToConsole("Failed to interpolate points");
			}
		}
	}
	ImGui::End();
	*/
}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void CMyApp::KeyboardDown(const SDL_KeyboardEvent& key)
{
	if (key.repeat == 0) // Triggers only once when held
	{
		if (key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL) // CTRL + F5
		{
			CleanShaders();
			InitShaders();
		}
		if (key.keysym.sym == SDLK_F1) // F1
		{
			GLint polygonModeFrontAndBack[2] = {};
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
			glGetIntegerv(GL_POLYGON_MODE, polygonModeFrontAndBack); // Query the current polygon mode. It gives the front and back modes separately.
			GLenum polygonMode = (polygonModeFrontAndBack[0] != GL_FILL ? GL_FILL : GL_LINE); // Switch between FILL and LINE
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
			glPolygonMode(GL_FRONT_AND_BACK, polygonMode); // Set the new polygon mode
		}
	}
	m_cameraManipulator.KeyboardDown(key);
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_cameraManipulator.KeyboardUp(key);
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_cursorPos = glm::vec2(mouse.x, mouse.y);
	m_cursorMoved = true;
	m_cameraManipulator.MouseMove(mouse);
}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void CMyApp::MouseDown(const SDL_MouseButtonEvent& mouse)
{
	m_cursorMoved = false;
}

void CMyApp::MouseUp(const SDL_MouseButtonEvent& mouse)
{
	if (!m_cursorMoved) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ModelIDBufferID);
		glm::vec4* activeObj = (glm::vec4*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		m_selectedModel = activeObj->x;
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void CMyApp::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
	m_cameraManipulator.MouseWheel(wheel);
}

// New window size
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.SetAspect(static_cast<float>(_w) / _h);
	m_width = _w;
	m_height = _h;
}

// Other SDL events
// https://wiki.libsdl.org/SDL2/SDL_Event

void CMyApp::OtherEvent(const SDL_Event& ev)
{
}