#pragma once

// C++ libraries
#include <filesystem>
#include <iterator>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <numeric>

// preprocessor config
#include "config.h"

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

// Log
#include "Log.h"

// Class list
#include "Classes.h"

// Interfaces
#include "Interfaces/IDrawable.h"
#include "Interfaces/IPrintable.h"

// Models
#include "Transformation.h"
#include "Material.h"
#include "Light.h"
#include "Types.h"
#include "Mesh.h"
#include "ModelLoader.h"
#include "ModelBase.h"
#include "Model.h"
#include "Splines/Bezier.h"
#include "Splines/BSpline.h"
#include "Splines/BSplineInterpolation.h"
#include "Surfaces/BezierSurface.h"

// main application
#include "MyApp.h"
