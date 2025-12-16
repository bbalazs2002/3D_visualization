#pragma once

// preprocessor config
#include "config.h"

// C++ libraries
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <math.h>
#include <format>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "SDL_GLDebugMessageCallback.h"

// Utils
#include "GLUtils.hpp"
#include "ObjParser.h"
#include "ProgramBuilder.h"
#include "Camera.h"
#include "CameraManipulator.h"

// ImGui
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

// TinyObjLoader
#include "tiny_obj_loader.h"

// Eigen
#include <Eigen/Dense>
#include <Eigen/LU>

// Log
#include "Log.h"

// Class list
#include "Classes.h"

// Interfaces
#include "Interfaces/IDrawable.h"
#include "Interfaces/IPrintable.h"

// Models
#include "Types.h"
#include "Transformation.h"
#include "Material.h"
#include "Lights/Light.h"
#include "Lights/DirectionalLight.h"
#include "Lights/PointLight.h"
#include "Lights/SpotLight.h"
#include "Models/Mesh.h"
#include "Models/ModelLoader.h"
#include "ModelBase.h"
#include "Models/Model.h"
#include "Curves/BezierCurve.h"
#include "Curves/BSpline.h"
#include "Curves/BSplineInterpolation.h"
#include "Curves/DiscreteCurve.h"
#include "Surfaces/BezierSurface.h"
#include "Surfaces/BezierSurfaceInterpolation.h"

// main application
#include "MyApp.h"
