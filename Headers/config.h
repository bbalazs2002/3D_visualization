#pragma once

#include "include_all.h"

// model - modelBase adapters
#define MODEL2MODELBASE ModelBaseParams{params.shaderPrograms,params.name,params.show,params.drawMode}
#define BEZIERCURVE2MODELBASE ModelBaseParams{params.shaderPrograms,params.name,params.show,GL_LINE_STRIP}
#define BSPLINE2MODELBASE ModelBaseParams{params.shaderPrograms,params.name,params.show,GL_LINE_STRIP}
#define DISCRETECURVE2MODELBASE ModelBaseParams{params.shaderPrograms,params.name,params.show,GL_LINE_STRIP}
#define BEZIERSURFACE2MODELBASE ModelBaseParams{params.shaderPrograms,params.name,params.show,GL_TRIANGLES}

// Light flags
#define LIGHT_FLAG_IS_DIR       (1u << 0) // 1
#define LIGHT_FLAG_IS_POINT     (1u << 1) // 2
#define LIGHT_FLAG_IS_SPOT      (1u << 2) // 4
#define LIGHT_FLAG_CASTS_SHADOW (1u << 3) // 8

// for <math.h>
#define _USE_MATH_DEFINES