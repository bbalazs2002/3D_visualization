#pragma once

#include "include_all.h"

// model type id
#define MODEL_TYPE_TYPE GLuint
#define MODEL_TYPE_MODELBASE 0
#define MODEL_TYPE_MODEL 1
#define MODEL_TYPE_BEZIER 2
#define MODEL_TYPE_BSPLINE 3
#define MODEL_TYPE_DISCRETECURVE 4
#define MODEL_TYPE_BEZIERSURFACE 5

// light type id
#define LIGHT_TYPE_TYPE GLint
#define LIGHT_TYPE_UNDEFINED -1
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

// model - modelBase adapters
#define MODEL2MODELBASE ModelBaseParams{params.programID,params.programSelectedID,params.name,params.show,params.drawMode}
#define BEZIERCURVE2MODELBASE ModelBaseParams{params.programID,params.programSelectedID,params.name,params.show,GL_LINE_STRIP}
#define BSPLINE2MODELBASE ModelBaseParams{params.programID,params.programSelectedID,params.name,params.show,GL_LINE_STRIP}
#define DISCRETECURVE2MODELBASE ModelBaseParams{params.programID,params.programSelectedID,params.name,params.show,GL_LINE_STRIP}
#define BEZIERSURFACE2MODELBASE ModelBaseParams{params.programID,params.programSelectedID,params.name,params.show,GL_TRIANGLES}

// for <math.h>
#define _USE_MATH_DEFINES